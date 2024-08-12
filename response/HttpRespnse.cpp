#include "HttpRespnse.hpp"

HttpResponse::HttpResponse() { }

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

void HttpResponse::setStatusCodeMsg(const std::string& msg)
{
	this->statusCodeMsg = msg;
}

void HttpResponse::setBody(const std::string& body)
{
	this->body = body;
}

std::string HttpResponse::generateResponse()
{
	std::string response = this->version + " " + std::to_string(this->statusCode) + " " + statusCodeMsg + "\r\n";
	for (auto it = this->headers.begin(); it != this->headers.end(); it++)
	{
		response += it->first + ": " + it->second + "\r\n";
	}
	response += "\r\n";
	response += this->body;
	return response;
}
