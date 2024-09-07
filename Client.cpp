#include "Client.hpp"

Client::~Client() {}

Client::Client() : fd(-1), writePending(false)
{
    lastActivity = time(NULL);
    env = NULL;
}

Client::Client(int fd, EventPoller *poller) : _poller(poller), fd(fd), writePending(false), readPending(false)
{
    lastActivity = time(NULL);
    env = NULL;
}

int Client::getFd() const { return this->fd; }

std::string& Client::getWriteBuffer() { return writeBuffer; }

bool Client::isWritePending() const { return writePending; }

bool Client::isReadPending() const { return readPending; }


void Client::setWritePending(bool pending) { writePending = pending; }

void Client::setReadPending(bool pending) { readPending = pending; }

void Client::setKeepAlive(bool keepAlive) { this->keepAlive = keepAlive; }

Request& Client::getRequest() { return request; }

Response& Client::getResponse() { return response; }


void Client::reset()
{
	writeBuffer.clear();
	writePending = false;
	readPending = false;
	request = Request();
	response = Response();
    cgi = Cgi();
    lastActivity = time(NULL);
}

bool Client::isKeepAlive()
{
	Headers &hdr = request.getHeaders();
	std::string connection = hdr.getValue("Connection");
    if (connection == "close")
		return false;
	return true;
}

Client::Client(const Client &c)
: fd(c.fd), writeBuffer(c.writeBuffer),
writePending(c.writePending), _poller(c._poller),
request(c.request), response(c.response) {}

Client& Client::operator=(const Client &c)
{
	if (this == &c)
		return *this;
	fd = c.fd;
	writeBuffer = c.writeBuffer;
	writePending = c.writePending;
	request = c.request;
    _poller = c._poller;
	response = c.response;
	return *this;
}

void Client::recvChunk()
{
    char buffer;
    std::string chunkSizeStr;
    int chunkSize = 0;
    std::string &bodyContent = this->request.getBody().getContent();

    while (true)
    {
        while (true)
        {
            if (recv(this->fd, &buffer, 1, 0) <= 0)
                throw std::runtime_error("[recvChunk]\t\t Error reading from client socket " + std::to_string(this->fd) + " " + strerror(errno));

            if (buffer == '\r')
            {
                // Expecting '\n' after '\r'
                if (recv(this->fd, &buffer, 1, 0) <= 0)
                    throw std::runtime_error("[recvChunk]\t\t Error reading from client socket " + std::to_string(this->fd) + " " + strerror(errno));
                if (buffer == '\n')
                    break;
                else
                    throw std::runtime_error("[recvChunk]\t\t Malformed chunk size");
            }
            chunkSizeStr.append(1, buffer);
        }

        // Convert chunk size from hex to integer
        try
        {
            chunkSize = std::stoi(chunkSizeStr, 0, 16);
        }
        catch (const std::exception &e)
        {
            throw std::runtime_error("[recvChunk]\t\t Error converting chunk size to integer");
        }

        if (chunkSize == 0)
        {
            if (recv(this->fd, &buffer, 1, 0) <= 0 || buffer != '\r')
                throw std::runtime_error("[recvChunk]\t\t Malformed final chunk");
            if (recv(this->fd, &buffer, 1, 0) <= 0 || buffer != '\n')
                throw std::runtime_error("[recvChunk]\t\t Malformed final chunk");
            break;
        }

        std::string chunkData(chunkSize, 0);
        int bytesRead = recv(this->fd, &chunkData[0], chunkSize, 0);
        if (bytesRead != chunkSize)
            throw std::runtime_error("[recvChunk]\t\t Error reading chunk data");
        bodyContent += chunkData;

        std::cout << "bodyContent: " << bodyContent << std::endl;

        // Read the trailing CRLF after the chunk data
        if (recv(this->fd, &buffer, 1, 0) <= 0 || buffer != '\r')
            throw std::runtime_error("[recvChunk]\t\t Malformed chunk data");
        if (recv(this->fd, &buffer, 1, 0) <= 0 || buffer != '\n')
            throw std::runtime_error("[recvChunk]\t\t Malformed chunk data");

        chunkSizeStr.clear();
    }

    Logs::appendLog("DEBUG", "[recvChunk]\t\t Chunked body received with size of " + std::to_string(bodyContent.size()));
}


void Client::recvHeader()
{
    std::string &buffer = this->request.getHeaders().getRawHeaders();
    int bytesRead;

    char c[1];
    while (buffer.find("\r\n\r\n") == std::string::npos)
    {
        bytesRead = recv(this->fd, c, 1, 0);
        if (bytesRead < 0)
            response.setErrorCode(500, "[recvHeader]\t\t Error reading from client socket");
        else if (bytesRead == 0)
            response.setErrorCode(499, "[recvHeader]\t\t Client disconnected " + std::to_string(this->fd));
        else
            buffer.append(c, bytesRead);
    }
    Logs::appendLog("DEBUG", "[recvHeader]\t\t Header received from client " + std::to_string(this->fd));
}

void Client::recvBody()
{
    std::string &buffer = this->request.getBody().getContent();
    int bytesRead;
    int contentLength = atoi(this->request.getHeaders().getValue("Content-Length").c_str());
    char c[1];

    while (buffer.size() < contentLength)
    {
        bytesRead = recv(this->fd, c, 1, 0);
        if (bytesRead < 0)
            response.setErrorCode(500, "[recvBody]\t\t Error reading from client socket");
        else if (bytesRead == 0)
            response.setErrorCode(499, "[recvBody]\t\t Client disconnected " + std::to_string(this->fd));
        else
            buffer.append(c, bytesRead);
    }
    Logs::appendLog("DEBUG", "[recvBody]\t\t Body received from client " + std::to_string(this->fd));
}

void Client::handleCGI(ServerConfig &serverConfig)
{
	std::string uri = request.getHeaders().getValue("uri");
	std::string fullPath = generateFullPath(serverConfig.root, uri);
    std::cout << "fullPath in CGI: " << fullPath << std::endl;
	this->cgi.execute(this->_poller, this->request, this->response, fullPath);
	std::string body = cgi.output;
	HttpResponse httpResponse;
	httpResponse.setVersion("HTTP/1.1");
	httpResponse.setStatusCode(200);
	httpResponse.setHeader("Content-Type", "text/html");
	httpResponse.setHeader("Content-Length", std::to_string(body.size()));
	httpResponse.setHeader("Connection", "keep-alive");
    std::string cookies = this->request.getHeaders().getValue("Cookie");
	if (cookies != "")
		httpResponse.setHeader("Set-Cookie", cookies);
	httpResponse.setHeader("Server", "LULUGINX");
	httpResponse.setBody(body);
    this->writeBuffer = httpResponse.generateResponse();
    this->writePending = true;
    this->readPending = false;
}

void Client::handleNormalResponse(ServerConfig &serverConfig)
{
    this->response.handleResponse(this->request);
    this->writeBuffer = this->response.getResponse();
    this->writePending = true;
    this->readPending = false;
}

void Client::readFromSocket(ServerConfig &serverConfig)
{
    try
    {
        Logs::appendLog("DEBUG", "[readFromSocket]\t\t Reading from client socket " + std::to_string(this->fd));
        recvHeader();
        this->request.getHeaders().parseHeader(this->response);

        if (this->request.isBodyExist(serverConfig, this->response, this->fd))
        {
            if (this->request.isChunked())
                recvChunk();
            else
                recvBody();
        }
        this->request.handleRequest(serverConfig, this->response);
        if (this->request.isComplete())
        {
            if (this->request.getIsCGI())
                handleCGI(serverConfig);
            else
                handleNormalResponse(serverConfig);
        }
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        Logs::appendLog("ERROR", e.what());
        handleNormalResponse(serverConfig);
    }
}

bool Client::isTimeOut()
{
    time_t now = time(NULL);
    if (now - lastActivity > CLIENT_TIMEOUT)
    {
        std::cout << "Client " << fd << " timed out" << std::endl;
        return true;
    }
    return false;
}