#!/bin/sh
find samples/apps/atw/ -name "*.h" -o -name "*.c" | xargs clang-format-9 -style=file -i
