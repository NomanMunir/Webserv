#include "Client.hpp"

Client::~Client() {}

Client::Client() : fd(-1), writePending(false) {}

Client::Client(int fd) : fd(fd), writePending(false), readPending(false) { }

int Client::getFd() const { return this->fd; }

std::string& Client::getReadBuffer() { return readBuffer; }

std::string& Client::getWriteBuffer() { return writeBuffer; }

bool Client::isWritePending() const { return writePending; }

void Client::setWritePending(bool pending) { writePending = pending; }

bool Client::isReadPending() const { return readPending; }

void Client::setReadPending(bool pending) { readPending = pending; }

void Client::setKeepAlive(bool keepAlive) { this->keepAlive = keepAlive; }

Request& Client::getRequest() { return request; }

Response& Client::getResponse() { return response; }

void Client::reset() {
	readBuffer.clear();
	writeBuffer.clear();
	writePending = false;
	readPending = false;
	request = Request();
	response = Response();
}

bool Client::isKeepAlive()
{
	Headers &hdr = request.getHeaders();
	std::string connection = hdr.getValue("Connection");
	if (connection == "keep-alive")
		return true;
	else if (connection == "close")
		return false;
	return false;
}

Client::Client(const Client &c) : fd(c.fd), readBuffer(c.readBuffer), writeBuffer(c.writeBuffer), writePending(c.writePending), request(c.request), response(c.response) {}

Client& Client::operator=(const Client &c) {
	if (this == &c)
		return *this;
	fd = c.fd;
	readBuffer = c.readBuffer;
	writeBuffer = c.writeBuffer;
	writePending = c.writePending;
	request = c.request;
	response = c.response;
	return *this;
}


void Client::recvChunk()
{
    char buffer;
    std::string chunkSize;
    std::string chunk;
    int size = 0;
	std::string &bodyContent = this->request.getBody().getContent();

    while (true)
    {
        if (recv(this->fd, &buffer, 1, 0) <= 0)
            throw std::runtime_error("Client::recvChunk: Error reading from client socket");
        if (buffer == '\r')
        {
            if (recv(this->fd, &buffer, 1, 0) <= 0)
                throw std::runtime_error("Client::recvChunk: Error reading from client socket");
            if (buffer == '\n')
            {
                size = std::stoi(chunkSize, 0, 16);
                if (size == 0)
                    break;
                while (size != chunk.size())
                {
                    if (recv(this->fd, &buffer, 1, 0) <= 0)
                        throw std::runtime_error("Client::recvChunk: Error reading from client socket");
                    chunk.append(1, buffer);
                }
                bodyContent += chunk;
                chunkSize.clear();
                chunk.clear();
            }
        }
        else
            chunkSize.append(1, buffer);
    }
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
            response.setErrorCode(500, "Client::recvHeader: Error reading from client socket");
        else if (bytesRead == 0)
            response.setErrorCode(400, "Client disconnected");
        else
            buffer.append(c, bytesRead);
    }
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
            response.setErrorCode(500, "Client::recvBody: Error reading from client socket");
        else if (bytesRead == 0)
            response.setErrorCode(400, "Client disconnected");
        else
            buffer.append(c, bytesRead);
    }
}

void Client::sendResponse()
{
	this->response.handleResponse(this->request, this->env);
	this->writeBuffer = this->response.getResponse();
    // std::cout << "Response: " << this->writeBuffer << std::endl;
	this->writePending = true;
	this->readPending = false;
	this->readBuffer.clear();

}

void Client::readFromSocket(Parser &configFile)
{
    try
    {
        recvHeader();
        this->request.getHeaders().parseHeader(this->response);

        if (this->request.isBodyExist(configFile, this->response))
        {
            if (this->request.isChunked())
                recvChunk();
            else
                recvBody();
        }
        this->request.handleRequest(configFile, this->response);
        if (this->request.isComplete())
            sendResponse();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        sendResponse();
    }
    
}
