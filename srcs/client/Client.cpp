#include "Client.hpp"

Client::Client() : _poller(NULL), fd(-1)
{
    lastActivity = time(NULL);
    writePending = false;
    readPending = false;
    keepAlive = false;
    env = NULL;
}

Client::Client(int fd, EventPoller *poller)
    : _poller(poller), fd(fd), writePending(false), \
    readPending(false), keepAlive(false)
{
    lastActivity = time(NULL);
    env = NULL;
    this->response.setClientSocket(fd);
}

Client::~Client() { }

Client::Client(const Client &c)
: _poller(c._poller), request(c.request), response(c.response), cgi(c.cgi), 
fd(c.fd), lastActivity(c.lastActivity), writeBuffer(c.writeBuffer),
writePending(c.writePending), readPending(c.readPending), keepAlive(c.keepAlive),
env(c.env) {}

Client& Client::operator=(const Client &c)
{
	if (this == &c)
		return *this;
    _poller = c._poller;
	request = c.request;
	response = c.response;
    cgi = c.cgi;
	fd = c.fd;
    lastActivity = c.lastActivity;
	writeBuffer = c.writeBuffer;
	writePending = c.writePending;
    readPending = c.readPending;
    keepAlive = c.keepAlive;
    env = c.env;
	return *this;
}

int Client::getFd() const { return this->fd; }

std::string& Client::getWriteBuffer() { return writeBuffer; }

bool Client::isWritePending() const { return writePending; }

bool Client::isReadPending() const { return readPending; }

Cgi& Client::getCgi() { return cgi; }


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

void Client::recvChunk()
{
    char buffer[1025];
    int bytesRead;
    std::string &bodyContent = this->request.getBody().getContent();
    while (true)
    {
        bytesRead = recv(this->fd, buffer, 1025, 0);
        if (bytesRead < 0)
            throw std::runtime_error("[recvChunk]\t\t Error reading from client socket " + intToString(this->fd) + " " + strerror(errno));
        else if (bytesRead == 0)
            throw std::runtime_error("[recvChunk]\t\t Client disconnected " + intToString(this->fd));
        bodyContent.append(buffer, bytesRead);
        if (bodyContent.find("0\r\n\r\n") != std::string::npos)
            break;
    }
    Logs::appendLog("DEBUG", "[recvChunk]\t\t Chunked body received with size of " + intToString(bodyContent.size()));
    this->request.getBody().isComplete() = true;
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
            response.setErrorCode(499, "[recvHeader]\t\t Client disconnected " + intToString(this->fd));
        else
            buffer.append(c, bytesRead);
    }
    this->request.getHeaders().isComplete() = true;
    Logs::appendLog("DEBUG", "[recvHeader]\t\t Header received from client " + intToString(this->fd));
}

void Client::recvBody()
{
    std::string &buffer = this->request.getBody().getContent();
    long int bytesRead;
    long unsigned int contentLength = atoi(this->request.getHeaders().getValue("Content-Length").c_str());
    char c[1];

    while (buffer.size() < contentLength)
    {
        bytesRead = recv(this->fd, c, 1, 0);
        if (bytesRead < 0)
            response.setErrorCode(500, "[recvBody]\t\t Error reading from client socket");
        else if (bytesRead == 0)
            response.setErrorCode(499, "[recvBody]\t\t Client disconnected " + intToString(this->fd));
        else
            buffer.append(c, bytesRead);
    }
    Logs::appendLog("DEBUG", "[recvBody]\t\t Body received from client " + intToString(this->fd));
    this->request.getBody().isComplete() = true;
}

void Client::validateCgiExtensions(std::vector<std::string> cgiExtensions, std::string fullPath)
{
    const long unsigned int pos = fullPath.find_last_of('.');
    if (pos == std::string::npos)
        this->response.setErrorCode(403, "[validateCgiExtensions]\t\t CGI extension not allowed");
    std::string extensions = fullPath.substr(pos);
	if (cgiExtensions.size() == 1 && cgiExtensions[0] == "")
        this->response.setErrorCode(403, "[validateCgiExtensions]\t\t CGI extension not allowed");
    for (size_t i = 0; i < cgiExtensions.size(); i++)
    {
        if (extensions == cgiExtensions[i])
            return;
    }
    this->response.setErrorCode(403, "[validateCgiExtensions]\t\t CGI extension not allowed");
}

void Client::handleCGI(ServerConfig &serverConfig)
{
	std::string uri = request.getHeaders().getValue("uri");
	std::string fullPath = generateFullPath(serverConfig.root, uri, "/");

    int type = response.checkType(fullPath);
    bool isDirListing = true;
    if (type == IS_DIR)
        response.handleDirectory(fullPath, uri, isDirListing);
    if (isDirListing)
    {
        validateCgiExtensions(serverConfig.cgiExtensions, fullPath);
        this->cgi.execute(this->_poller, this->request, this->response, fullPath);
    }
    else
    {
        this->writeBuffer = this->response.getResponse();
        this->writePending = true;
        this->readPending = false;
    }
}

void Client::handleNormalResponse()
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
        Logs::appendLog("DEBUG", "[readFromSocket]\t\t Reading from client socket " + intToString(this->fd));
        while(!this->request.getHeaders().isComplete())
            recvHeader();
        this->request.getHeaders().parseHeader(this->response);

        if (this->request.isBodyExist(serverConfig, this->response))
        {
            Logs::appendLog("DEBUG", "[readFromSocket]\t\t Reading body from client socket " + intToString(this->fd));
            while(!this->request.getBody().isComplete())
            {
                if (this->request.isChunked())
                    recvChunk();
                else
                    recvBody();
            }
        }
        this->request.handleRequest(serverConfig, this->response);
        if (this->request.isComplete())
        {
            if (this->request.getIsCGI())
                handleCGI(serverConfig);
            else
                handleNormalResponse();
        }
    }
    catch(const std::exception& e)
    {
        Logs::appendLog("ERROR", e.what());
        handleNormalResponse();
    }
}

bool Client::isTimeOut()
{
    time_t now = time(NULL);
    if (now - lastActivity > CLIENT_TIMEOUT)
        return true;
    return false;
}

void Client::updateLastActivity()
{
    this->lastActivity = time(NULL);
}

bool Client::isCGI() const { return this->cgi.getIsCGI(); }