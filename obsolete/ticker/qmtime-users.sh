#!/bin/bash

KEYWORD="QmTime"

function grep_it
{
  objdump --demangle -T $1 2>/dev/null | grep $2
}

FILES=$(find /usr/lib -name \*.so\* -and -type f)
FILES="$FILES $(find /usr/bin -type f)"

for file in $FILES ; do
    if grep_it "$file" "$KEYWORD" >/dev/null
    then
       dpkgS=$(dpkg -S $file)
       binary=`echo "$dpkgS" | sed s/\:.\*//g`
       source=$(apt-cache show "$binary" | grep "^Source:" | awk '{print $2}')
       test -z "$source" && source="$binary"
       echo "file='$file' binary='$binary' source='$source'"
       grep_it "$file" "$KEYWORD"
       echo "-------------------------------------------------------"
    fi
done





