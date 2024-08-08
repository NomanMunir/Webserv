#!/bin/bash

# URL to send the request to
URL="http://localhost:80"

# Function to generate a chunk of specified size
generate_chunk() {
    local size=$1
    printf "%x\r\n" $size
    head -c $size /dev/zero | tr '\0' 'a'
    printf "\r\n"
}

# Prepare the chunked data
{
    # Generate first chunk of 10 bytes
    generate_chunk 10

    # Generate second chunk of 20 bytes
    generate_chunk 20

    # Final chunk (0 bytes to indicate the end)
    printf "0\r\n\r\n"
} | curl -v -X GET -H "Transfer-Encoding: chunked" --data-binary @- "$URL"
