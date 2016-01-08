#!/bin/bash

find ../src -name "*.[ch]" | xargs awk '/TODO:/ {todos+=1;sub(/[ \t]+/, "", $0);print todos" "$0" "FILENAME}'
