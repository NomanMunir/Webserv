#include "Client.hpp"

Client::~Client() {}

Client::Client() : clientFd(-1), writePending(false) {}

Client::Client(int fd) : clientFd(fd), writePending(false), readPending(false) { }

int Client::getClientFd() const { return clientFd; }

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

bool Client::isKeepAlive() {
	std::string connection = request.getHeaders().getValue("Connection");
	if (connection == "keep-alive")
		return true;
	else if (connection == "close")
		return false;
	return false;
}

Client::Client(const Client &c) : clientFd(c.clientFd), readBuffer(c.readBuffer), writeBuffer(c.writeBuffer), writePending(c.writePending), request(c.request), response(c.response) {}

Client& Client::operator=(const Client &c) {
	if (this == &c)
		return *this;
	clientFd = c.clientFd;
	readBuffer = c.readBuffer;
	writeBuffer = c.writeBuffer;
	writePending = c.writePending;
	request = c.request;
	response = c.response;
	return *this;
}
