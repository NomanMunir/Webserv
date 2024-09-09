#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <map>
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
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;
};

#endif // "HTTPRESPONSE_HPP"