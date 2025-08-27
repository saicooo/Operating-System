#!/bin/bash

[ $# -ne 1 ] && echo "$0: Error: No file specified." && exit 1

output_file="$1"
> "$output_file"

foo() {
    sudo ls -lR / 2>/dev/null | awk -v type="$1" '
    $1 ~ "^"type {
        file_path = (dir == "/" ? "" : dir) "/" $NF;
        printf "%s %s\n", $1, file_path;
        exit;
    }' >> "$output_file"
}

TYPES=("-" "b" "c" "d" "l" "p" "s")

for type in ${TYPES[@]}; do
    foo "$type"
done

echo "Results saved in '$output_file'."
