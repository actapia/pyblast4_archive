#!/usr/bin/bash
function find_dep {
    shopt -s nullglob
    toolkits=("$1"*"$3")
    if [ "${#toolkits[@]}" -lt 1 ]; then
	>&2 echo "Could not find $2. Was it installed?"
	exit 1
    elif [ "${#toolkits[@]}" -gt 1 ]; then
	>&2 echo "Multiple $2 installations found. Refusing to pick."
	exit 1
    fi
    echo "${toolkits[0]}"
}
set -e
toolkit="$(find_dep /opt/ncbi-cxx-toolkit- 'NCBI C++ Toolkit')"
toolkit_lib="$(find_dep "$toolkit"/lib 'NCBI C++ Toolkit library dir')"
toolkit_include="$(find_dep "$toolkit"/include 'NCBI C++ Toolkit include dir')"
bpy="$(find_dep /opt/boost-python- 'boost::python')"
bpy_lib="$(find_dep "$bpy"/lib 'boost::python library dir')"
bpy_include="$(find_dep "$bpy"/include 'boost::python include dir')"
echo "NCBI C++ Toolkit: $toolkit"
echo "boost::python: $bpy"
set -x
for pydir in /opt/python/*; do
    if ! version="$(
       "$pydir/bin/python" ci/requires_version_okay.py pyproject.toml
    )"; then
	continue
    fi
    #echo "$version is okay"
    version_name="$(basename "$pydir")"
    echo "$version $version_name"
    if ! [ "$bpy_lib"/"libboost_python$version-$version_name.so" ]; then
    	>&2 echo "Could not find libboost_python for version $version_name."
    	exit 1
    fi
    echo "Building for $version_name"
    python_include="$(find_dep "$pydir"/include/ "$version_name include dir" /)"
    CXXFLAGS=("-I$toolkit_include/ncbi-tools++"
    	      "-I$bpy_include"
    	      "-I$python_include")
    LDFLAGS=("-L$toolkit_lib"
    	     "-L$bpy_lib"
	     "-lboost_python$version-$version_name"
	     "-Wl,-rpath"
	     "-Wl,$toolkit_lib"
	     "-Wl,-rpath"
	     "-Wl,$bpy_lib")
    CXXFLAGS="${CXXFLAGS[*]}" LDFLAGS="${LDFLAGS[*]}" \
    	    "$pydir/bin/python" -m pip wheel . -w /root/wheelhouse
    echo "Testing package can be imported on $version_name"
    "$pydir/bin/python" -m pip install /root/wheelhouse/*-"$version_name"*.whl
    "$pydir/bin/python" -c 'import pyblast4_archive'
done

