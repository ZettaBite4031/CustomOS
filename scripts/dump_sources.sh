#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <folder>"
    exit 1
fi

FOLDER="$1"
OUTPUT="output.txt"

> "$OUTPUT"

find "$FOLDER" -type f \( -name "*.cpp" -o -name "*.hpp" \) | while read -r FILE; do
    echo "------- $FILE -------" >> "$OUTPUT"
    cat "$FILE" >> "$OUTPUT"
    echo -e "\n" >> "$OUTPUT"
done
