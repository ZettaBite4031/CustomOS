#!/bin/bash

# Extensions to include
EXTENSIONS=("c" "cpp" "h" "hpp")

# Construct the find command with the proper -name conditions
FIND_ARGS=()
for ext in "${EXTENSIONS[@]}"; do
    FIND_ARGS+=(-name "*.${ext}" -o)
done
# Remove the last -o
unset 'FIND_ARGS[${#FIND_ARGS[@]}-1]'

# Find files and count lines
LINE_COUNT=$(find . \( "${FIND_ARGS[@]}" \) -type f -print0 | xargs -0 cat | wc -l)

echo -e "\033[1;31mZOS Source Code Length: \033[1;33m$LINE_COUNT \033[0m"