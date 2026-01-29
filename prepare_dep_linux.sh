#!/usr/bin/env bash
set -e
set -x
rpmbuild_dir="$HOME/rpmbuild"
mkdir -p deps
rm -rf "$rpmbuild_dir"
tmp="$(mktemp -d)"
yumdownloader --source "$1" --dest "$tmp"
spec="$(rpm -q -l "$tmp"/*.src.rpm | grep '\.spec$')"
name="$(rpm -q "$tmp"/*.src.rpm --qf '%{NAME}\n')"
rpm --install "$tmp"/*.src.rpm
rpmbuild -bp --nodeps "$rpmbuild_dir/SPECS/$spec"
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
# mkdir "deps/$name"
# mv /root/rpmbuild/SPECS/* "deps/$name/"
# mv /root/rpmbuild/SOURCES/* "deps/$name/"
