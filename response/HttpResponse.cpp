#include "HttpResponse.hpp"

HttpResponse::HttpResponse()
{
	this->setHeader("Server", "LULUGINX");
	this->setHeader("Connection", "keep-alive");
}

HttpResponse::~HttpResponse() { }

HttpResponse::HttpResponse(const HttpResponse& other)
{
	this->statusCode = other.statusCode;
	this->headers = other.headers;
	this->body = other.body;
}

HttpResponse& HttpResponse::operator=(const HttpResponse& other)
{
	this->statusCode = other.statusCode;
	this->headers = other.headers;
	this->body = other.body;
	return (*this);
}

void HttpResponse::setStatusCode(int code)
{
	this->statusCode = code;
}

void HttpResponse::setVersion(const std::string& version)
{
	this->version = version;
}

void HttpResponse::setHeader(const std::string& key, const std::string& value)
{
	this->headers[key] = value;
}

void HttpResponse::setBody(const std::string& body)
{
	this->body = body;
}


std::string HttpResponse::generateResponse()
{
	std::string response = this->version + " " + std::to_string(this->statusCode) + " " + getStatusMsg(std::to_string(this->statusCode)) + "\r\n";
	for (std::map<std::string, std::string>::iterator it = this->headers.begin(); it != this->headers.end(); it++)
		response += it->first + ": " + it->second + "\r\n";
	response += "\r\n";
	response += this->body;
	return response;
}
