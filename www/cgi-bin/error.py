#!/usr/bin/env python3

import os
import sys

def main():
    # Check if the request method is POST
    if os.environ.get('REQUEST_METHOD', '') == 'POST'
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
