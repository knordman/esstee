#!/bin/sh

find ../src -name "*.[ylch]" -print | awk '!/bison.tab.c|flex.c|testground/' | xargs wc --lines | sort -n
