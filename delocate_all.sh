#!/usr/bin/env zsh
export DYLD_LIBRARY_PATH="$(brew --prefix ncbi-cxx-toolkit)/lib:$(brew --prefix boost-python-cibuildwheel)/lib"
set -x
set -e
for f in "$3"/*.whl; do
    "$1" -m delocate.cmd.delocate_wheel -w "$2" "$f"
done
