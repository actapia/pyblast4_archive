#!/usr/bin/env zsh
set -e
set -x
mkdir -p deps/"$1"
res="$(zsh get_package_source.sh "$1")"
mv "$res" deps/"$1"/
cp "$(brew edit --print-path "$1")" deps/"$1"/
