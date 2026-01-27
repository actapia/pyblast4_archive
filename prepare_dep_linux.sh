#!/usr/bin/env bash
set -e
set -x
mkdir -p deps
rm -rf /root/rpmbuild
tmp="$(mktemp -d)"
yumdownloader --source "$1" --dest "$tmp"
spec="$(rpm -q -l "$tmp"/*.src.rpm | grep '\.spec$')"
name="$(rpm -q "$tmp"/*.src.rpm --qf '%{NAME}\n')"
rpm --install "$tmp"/*.src.rpm
rpmbuild -bp --nodeps "/root/rpmbuild/SPECS/$spec"
# build_dirs=(/root/rpmbuild/BUILD/*)
# if [ "${#build_dirs[@]}" -eq 1 ]; then
#     mv "${build_dirs[0]}" "deps/$name"
# else
mv /root/rpmbuild/BUILD "deps/$name"
# fi
mv "/root/rpmbuild/SPECS/$spec" "deps/$name/"
mv /root/rpmbuild/SOURCES/ "deps/$name/patches"
shopt -s nullglob
rm "deps/$name/patches/"*.zip "deps/$name/patches/"*.tar*
# mkdir "deps/$name"
# mv /root/rpmbuild/SPECS/* "deps/$name/"
# mv /root/rpmbuild/SOURCES/* "deps/$name/"
