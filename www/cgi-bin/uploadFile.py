#!/usr/bin/env python3

import cgi
import os
import random
import string

def generate_random_filename(length=8, extension=''):
    characters = string.ascii_letters + string.digits
    filename = ''.join(random.choice(characters) for _ in range(length)) + extension
    return filename

def main():
    # Set the maximum size of the uploaded file (e.g., 10 MB)
    cgi.maxlen = 10 * 1024 * 1024

    # Create a FieldStorage instance to parse the form data
    form = cgi.FieldStorage()

    # Check if the file was uploaded
    if 'file' in form and form['file'].filename:
        file_item = form['file']

        # Extract the filename and file data
        original_filename = os.path.basename(file_item.filename)
        file_data = file_item.file.read()

        # Generate a random filename, preserving the original file extension
        _, file_extension = os.path.splitext(original_filename)
        random_filename = generate_random_filename(extension=file_extension)

        # Specify the directory where the uploaded files will be saved
        upload_dir = '../uploads'  # Change this to your desired directory

        # Ensure the upload directory exists
        if not os.path.exists(upload_dir):
            os.makedirs(upload_dir)

        # Save the file
        file_path = os.path.join(upload_dir, random_filename)
        print(f"Saving file to: {file_path}")
        with open(file_path, 'wb') as f:
            f.write(file_data)

        # Output HTML page with centered message
        print("<html>")
        print("<head><title>File Uploaded</title></head>")
        print("<body>")
        print("<div style='text-align: center; margin-top: 50px;'>")
        print(f"<h1>File '{original_filename}' has been uploaded as '{random_filename}'</h1>")
        print(f"<p><a href=../uploads/{random_filename}>read</a></p>")
        print("</div>")
        print("</body>")
        print("</html>")
    else:
        # No file was uploaded
        print("<html>")
        print("<head><title>No File Uploaded</title></head>")
        print("<body>")
        print("<div style='text-align: center; margin-top: 50px;'>")
        print("<h1>No file was uploaded. Please go back and choose a file to upload.</h1>")
        print("</div>")
        print("</body>")
        print("</html>")

if __name__ == "__main__":
    main()
