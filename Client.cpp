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

void Client::reset()
{
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

Client& Client::operator=(const Client &c)
{
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
    std::string chunkSizeStr;
    int chunkSize = 0;
    std::string &bodyContent = this->request.getBody().getContent();

    while (true)
    {
        // Read chunk size
        while (true)
        {
            if (recv(this->fd, &buffer, 1, 0) <= 0)
                throw std::runtime_error("Client::recvChunk: Error reading from client socket");

            if (buffer == '\r')
            {
                // Expecting '\n' after '\r'
                if (recv(this->fd, &buffer, 1, 0) <= 0)
                    throw std::runtime_error("Client::recvChunk: Error reading from client socket");
                if (buffer == '\n')
                    break;
                else
                    throw std::runtime_error("Client::recvChunk: Malformed chunk size");
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
            throw std::runtime_error("Client::recvChunk: Invalid chunk size");
        }
        std::cout << "Chunk size: " << chunkSize << std::endl;

        // If chunk size is 0, it means the end of the chunks
        if (chunkSize == 0)
        {
            // Read the final CRLF after the last chunk
            if (recv(this->fd, &buffer, 1, 0) <= 0 || buffer != '\r')
                throw std::runtime_error("Client::recvChunk: Malformed final chunk");
            if (recv(this->fd, &buffer, 1, 0) <= 0 || buffer != '\n')
                throw std::runtime_error("Client::recvChunk: Malformed final chunk");
            break;
        }

        // Read the chunk data
        std::string chunkData(chunkSize, 0);
        int bytesRead = recv(this->fd, &chunkData[0], chunkSize, 0);
        if (bytesRead != chunkSize)
            throw std::runtime_error("Client::recvChunk: Error reading chunk data");
        bodyContent += chunkData;

        std::cout << "bodyContent: " << bodyContent << std::endl;

        // Read the trailing CRLF after the chunk data
        if (recv(this->fd, &buffer, 1, 0) <= 0 || buffer != '\r')
            throw std::runtime_error("Client::recvChunk: Malformed chunk data");
        if (recv(this->fd, &buffer, 1, 0) <= 0 || buffer != '\n')
            throw std::runtime_error("Client::recvChunk: Malformed chunk data");

        // Clear the chunk size string for the next chunk
        chunkSizeStr.clear();
    }

    std::cout << "Received chunked data successfully" << std::endl;
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

        if (this->request.isBodyExist(configFile, this->response, this->fd))
        {
            if (this->request.isChunked())
                recvChunk();
            else
                recvBody();
        }
        std::cout << "Body:: " << this->request.getBody().getContent() << std::endl;
        this->request.handleRequest(configFile, this->response);
        if (this->request.isComplete())
        {
            std::cout << "Request is complete" << std::endl;
            sendResponse();
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << "sds\n";
        sendResponse();
    }    
}
