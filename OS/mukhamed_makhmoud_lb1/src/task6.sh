!/bin/bash

FILE="$1"

[ $# -ne 1 ] && echo "$0: Error: No file specified." && exit 1

echo "Analyzing $FILE via od:"
od -A x -t x1z -N 64 "$FILE"
echo "----------------------------------------"

echo "Analyzing $FILE via hexdump:"
hexdump -C -n 64 "$FILE"
echo "----------------------------------------"