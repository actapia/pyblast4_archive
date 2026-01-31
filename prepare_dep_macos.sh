#!/usr/bin/env zsh
set -e
set -x
mkdir -p deps/"$1"
brew get-source --out deps/"$1" "$1" >&2
cp "$(brew edit --print-path "$1")" deps/"$1"/
license="$(brew info --json "$1" | jq -r '.[0] | .license')"
version="$(brew info "$1" | jq -r '.[0] | .versions.stable')"
echo "$1 $version: $license"
