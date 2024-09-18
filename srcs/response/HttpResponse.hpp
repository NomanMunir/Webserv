#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <map>
#include <string>
#include "../utils/utils.hpp"

class HttpResponse
{
public:
	HttpResponse();
	~HttpResponse();
	HttpResponse(const HttpResponse&);
	HttpResponse& operator=(const HttpResponse&);

	void setStatusCode(int code);
	void setVersion(const std::string& version);
	void setHeader(const std::string& key, const std::string& value);
	void setBody(const std::string& body);
	std::string generateResponse();

private:
	int statusCode;
	std::string body;
	std::string version;
	std::map<std::string, std::string> headers;
};

#endif // "HTTPRESPONSE_HPP"