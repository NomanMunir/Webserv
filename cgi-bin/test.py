#!/usr/bin/env python3

import os
import sys
import urllib.parse
import cgi  # Import the cgi module

def handle_get_request():
    # Get the query string
    query_string = os.getenv('QUERY_STRING', '')

    # Parse the query string into a dictionary
    params = urllib.parse.parse_qs(query_string

    # Generate HTML response
    print()
    print("<html><body>")
    print("<h1>GET Request Received</h1>")
    print("<h2>Parameters:</h2>")
    for key, values in params.items():
        for value in values:
            print(f"<p>{key}: {value}</p>")
    print("</body></html>")

def handle_post_request():
    # Create an instance of FieldStorage
    form = cgi.FieldStorage()

    # Generate HTML response
    print()
    print("<html><body>")
    print("<h1>POST Request Received</h1>")
    print("<h2>Form Data:</h2>")

    # Iterate over the form fields
    for key in form.keys():
        item = form[key]
        if item.filename:  # This is a file upload
            print(f"<p>File Field: {key}, Filename: {item.filename}</p>")
            # Read file content (if necessary)
            file_content = item.file.read()
            print(f"<p>File Content: {file_content.decode('utf-8')}</p>")
        else:  # Regular form field
            print(f"<p>{key}: {item.value}</p>")
    print("</body></html>")

def main():
    # Determine the request method
    request_method = os.environ.get('REQUEST_METHOD', '')
    print("Method:", request_method, file=sys.stderr)
    if request_method == 'GET':
        handle_get_request()
    elif request_method == 'POST':
        handle_post_request()
    else:
        print("Content-Type: text/html")
        print()
        print("<html><body>")
        print("<h1>Unsupported HTTP Method</h1>")
        print("</body></html>")

if __name__ == "__main__":
    main()
