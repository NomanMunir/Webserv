#!/bin/bash

while (true); do
    echo "This is an infinite loop"
    sleep 1  # Pause for 1 second to avoid high CPU usage
done

# i=0
# while [ $i -lt 5 ]; do
#     echo "This is iteration $i"
#     sleep 1  # Pause for 1 second
#     i=$((i + 1))
# done