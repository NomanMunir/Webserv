#!/usr/bin/env python3

import os

# Print necessary CGI headers
print("Content-Type: text/html")
# print()

# # Accessing QUERY_STRING environment variable
# query_string = os.getenv('QUERY_STRING', '')

# # Parsing QUERY_STRING manually
# params = {}
# if query_string:
#     pairs = query_string.split('&')
#     for pair in pairs:
#         key, value = pair.split('=')
#         params[key] = value

# # Extract parameters
# name = params.get('name', 'Unknown')
# age = params.get('age', 'Unknown')

# Generate HTML response
# print("<html><body>")
# print("<h1>CGI Script Output</h1>")
# print(f"<p>Name: {name}</p>")
# print(f"<p>Age: {age}</p>")
# print("</body></html>")
