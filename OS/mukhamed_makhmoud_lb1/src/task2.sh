#!/bin/bash

if [ $# -lt 1 ]; then
  echo "Usage: $0 /path/to/file"
  exit 1
fi

goal="$1"
output_file="output.txt"
> "$output_file"

inode=$(stat -c '%i' "$goal")

ls -Ri ~ 2>/dev/null | awk -v inode="$inode"  '
  /:$/ { dir = substr($0, 1, length($0)-1); next }
  $1 == inode { sub(/^[[:space:]]+/, "", $0); print dir "/" $0 }
' >> "$output_file"