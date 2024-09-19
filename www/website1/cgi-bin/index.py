#!/usr/bin/env python3

print("""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Welcome to My Pretty CGI World</title>
    <style>
        body {
            background-color: pink;
            font-family: Arial, sans-serif;
            text-align: center;
            color: white;
        }
        h1 {
            margin-top: 20%;
            font-size: 3rem;
            text-shadow: 2px 2px 5px rgba(0, 0, 0, 0.2);
        }
        p {
            font-size: 1.5rem;
            color: #fff;
        }
        a {
            color: white;
            text-decoration: none;
            font-weight: bold;
        }
        a:hover {
            color: #ffc0cb;
        }
    </style>
</head>
<body>
    <h1>Welcome to My Pretty CGI World!</h1>
    <p>This is a simple HTML page generated by a Python CGI script.</p>
    <p>Click <a href="/">here</a> to go back home.</p>
</body>
</html>
""")