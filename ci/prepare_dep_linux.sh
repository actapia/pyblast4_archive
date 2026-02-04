#!/usr/bin/env bash
set -e
set -x
rpmbuild_dir="$HOME/rpmbuild"
mkdir -p deps
rm -rf "$rpmbuild_dir"
tmp="$(mktemp -d)"
yumdownloader "$1" --dest "$tmp" >&2
original_name="$(rpm -q "$tmp"/*.rpm --qf '%{NAME}\n' | head -n1)"
yumdownloader --source "$1" --dest "$tmp" >&2
spec="$(rpm -q -l "$tmp"/*.src.rpm | grep '\.spec$')"
name="$(rpm -q "$tmp"/*.src.rpm --qf '%{NAME}\n')"
version="$(rpm -q "$tmp"/*.src.rpm --qf '%{VERSION}\n')"
license="$(rpm -q "$tmp"/*.src.rpm --qf '%{LICENSE}\n')"
rpm --install "$tmp"/*.src.rpm >&2
rpmbuild -bp --nodeps "$rpmbuild_dir/SPECS/$spec" >&2
# build_dirs=(/root/rpmbuild/BUILD/*)
# if [ "${#build_dirs[@]}" -eq 1 ]; then
#     mv "${build_dirs[0]}" "deps/$name"
# else
mv "$rpmbuild_dir/BUILD" "deps/$name"
# fi
mv "$rpmbuild_dir/SPECS/$spec" "deps/$name/"
mv "$rpmbuild_dir/SOURCES/" "deps/$name/patches"
shopt -s nullglob
rm "deps/$name/patches/"*.zip "deps/$name/patches/"*.tar*
rm -rf "$tmp"
echo -n "$name "
if [ "$name" != "$original_name" ]; then
    echo -n "($original_name) "
fi
echo "$version: $license"
# mkdir "deps/$name"
# mv /root/rpmbuild/SPECS/* "deps/$name/"
# mv /root/rpmbuild/SOURCES/* "deps/$name/"
