#!/usr/bin/env zsh
tmp="$(mktemp -d)"
mkfifo "$tmp/fifo"
brew reinstall -s -i "$1" <<< "pwd > $tmp/fifo; exit 12" >&2 &
brew_pid=$!
res="$(cat "$tmp/fifo")"
wait "$brew_pid"
rm -r "$tmp"
echo "$res"
