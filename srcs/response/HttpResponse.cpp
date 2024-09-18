#include "HttpResponse.hpp"

HttpResponse::HttpResponse() { }

HttpResponse::~HttpResponse() { }

HttpResponse::HttpResponse(const HttpResponse& other)
{
	this->statusCode = other.statusCode;
	this->body = other.body;
	this->version = other.version;
	this->headers = other.headers;
}

HttpResponse& HttpResponse::operator=(const HttpResponse& other)
{
	this->statusCode = other.statusCode;
	this->body = other.body;
	this->version = other.version;
	this->headers = other.headers;
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
	std::string response = this->version + " " + intToString(this->statusCode) + " " + getStatusMsg(intToString(this->statusCode)) + "\r\n";
	for (std::map<std::string, std::string>::iterator it = this->headers.begin(); it != this->headers.end(); it++)
		response += it->first + ": " + it->second + "\r\n";
	response += "\r\n";
	response += this->body;
	return response;
}
