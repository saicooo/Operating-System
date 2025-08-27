#!/bin/bash

[ $# -ne 1 ] && echo "$0: Error: No file specified." && exit 1
target_file="$1"
output_file="symb_links.txt"


> "$output_file"

{
  echo "Searching by find and stat:"
  find ~ -type l -exec stat -c "%N" {} \; 2>/dev/null | grep "$target_file"
  echo -e "\n"
} >> "$output_file"

{
  echo "Searching by find and -lname:"
  find ~ -type l -lname "*$target_file*" 2>/dev/null
  echo -e "\n"
} >> "$output_file"

{
  echo "Searching by readlink in cycle:"
  for link in $(find ~ -type l 2>/dev/null); do
    if readlink "$link" | grep -q "$target_file"; then
      echo "$link -> $(readlink "$link")"
    fi
  done
  echo -e "\n"
} >> "$output_file"

{
  echo "Searching by ls and grep:"
  ls -lRa ~ | grep " -> " | grep "$target_file"
  echo -e "\n"
} >> "$output_file"