#!/usr/bin/env zsh
set -e
set -x
mkdir -p deps/"$1"
brew get-source --out deps/"$1" "$1"
cp "$(brew edit --print-path "$1")" deps/"$1"/
