#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR/..
for_developers/format_all_source_files.sh
git add -u
git commit
