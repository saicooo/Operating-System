#!/bin/sh

[ $# -ne 1 ] && echo "$0: Error: No file specified." && exit 1

output_file="./file_links.txt"

> "$output_file"

ls -lR /home | grep $1 | grep ^l | awk '{print $0}' > "$output_file"  

cnt=$(wc -l < "$output_file")

echo "total $cnt" >> "$output_file"

# Вывод результата
echo "Result saved in '$output_file'"
echo "Symbol links found: $cnt"