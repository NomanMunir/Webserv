#!/bin/bash
echo "Content-Type: text/plain"
echo ""

if [ "$REQUEST_METHOD" = "GET" ]; then
    echo "GET request received"
    echo "Query: $QUERY_STRING"
elif [ "$REQUEST_METHOD" = "POST" ]; then
    echo "POST request received"
    read -n $CONTENT_LENGTH POST_DATA
    echo "Data: $POST_DATA"
fi
