#!/bin/bash

# URL to send the request to
URL="http://localhost:80"

# Function to generate a chunk of specified size with specified content
generate_chunk() {
    local size=$1
    local content=$2
    printf "%x\r\n" $size        # Print the chunk size in hexadecimal
    printf "%s\r\n" "$content"   # Print the chunk content followed by a CRLF
}

# Prepare and send the chunked data using curl
{
    # Generate first chunk of 10 bytes
    generate_chunk 10 "aaaaaaaaaa"

    # Generate second chunk of 20 bytes
    generate_chunk 20 "aaaaaaaaaaaaaaaaaaaa"

    # Final chunk (0 bytes to indicate the end)
    printf "0\r\n\r\n"
} | curl -v -X GET -H "Transfer-Encoding: chunked" --data-binary @- "$URL"
