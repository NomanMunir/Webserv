#include "../response/Response.hpp"

Request::Request() : complete(false) {}

bool Request::appendData(const std::string &data, Response &response, Parser &configFile) 
{
    rawData += data;
    if (!complete) {
		std::cout << "Request data: " << rawData << std::endl;
        for (int i = 0; i < rawData.size(); i++)
        {
            if (!isascii(rawData[i]))
                response.sendError("400");
        }
        handleRequest(response, configFile);
		// this->headers = Headers(rawData, response);
        // if (headers.isComplete()) 
        // {
        //     complete = true;  // Or set some condition for complete request
        // //     parseBody();
        // }
    }
    return complete;
}

bool Request::isComplete() const {
    return complete;
}

Headers Request::getHeaders() {
    return headers;
}

Body Request::getBody() {
    return body;
}

Request::~Request() {}
Request::Request(const Request &c) : rawData(c.rawData), headers(c.headers), body(c.body), complete(c.complete) {}
Request& Request::operator=(const Request &c) {
    if (this != &c) {
        rawData = c.rawData;
        headers = c.headers;
        body = c.body;
        complete = c.complete;
    }
    return *this;
}
