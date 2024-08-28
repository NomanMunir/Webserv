#!/usr/bin/env python3

# import os

# # Print necessary CGI headers
# print("Content-Type: text/html")
# print()

# # # Accessing QUERY_STRING environment variable
# query_string = os.getenv('QUERY_STRING', '')

# # Parsing QUERY_STRING manually
# # params = {}
# # if query_string:
# #     pairs = query_string.split('&')
# #     for pair in pairs:
# #         key, value = pair.split('=')
# #         params[key] = value

# # # Extract parameters
# # name = params.get('a', 'Unknown')
# # age = params.get('n', 'Unknown')

# # Generate HTML response
# print("<html><body>")
# print("<h1>CGI Script Output</h1>")
# # print(f"<p>Name: {query_string}</p>")
# # print(f"<p>Name: {name}</p>")
# # print(f"<p>Age: {age}</p>")
# # print("</body></html>")


#!/usr/bin/env python3

import os
import sys

def main():
    # Check if the request method is POST
    if os.environ.get('REQUEST_METHOD', '') == 'POST':
        # # Get the content length
        content_length = int(os.environ.get('CONTENT_LENGTH', 0))

        # # Read the body of the POST request
        body = sys.stdin.read(content_length)

        # Print the body
        print("Content-Type: text/plain\n")
        print("Received POST data:\n")
        # print(body)
    else:
        query_string = os.getenv('QUERY_STRING', '')
        print("<h1>CGI Script Output</h1>")
        print(f"<p>Name: {query_string}</p>")
        print("Content-Type: text/plain\n")
if __name__ == "__main__":
    main()
