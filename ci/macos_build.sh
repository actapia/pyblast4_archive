#!/usr/bin/env zsh
function find_dep {
    if [ "$#" -gt 2 ]; then
	>&2 echo "Multiple $1 installations found. Refusing to pick."
	exit 1
    elif ! [ -e "$2" ]; then
	>&2 echo "Could not find $1. Was it installed?"
	exit 1
    fi
    echo "$2"
}

function build {
    python_bin="python"
    if [ "$#" -eq 3 ]; then
	python_bin="$3"
    fi
    if ! version="$(
       "$1/bin/$python_bin" ci/requires_version_okay.py pyproject.toml
    )"; then
	return
    fi
    version_name="$(printf "$2" "$version")"
    # if ! [ -f "$bpy_lib"/"libboost_python$version-$version_name.dylib" ]; then
    # 	>&2 echo "Could not find libboost_python for version $2."
    # 	exit 1
    # fi
    lib_path="$(find_dep \
                "boost::python library for $version" \
                "$bpy_lib"/"libboost_python$version-"${~version_name}".dylib")"
    lib_name="$(basename "$lib_path")"
    lib_name="${lib_name#lib}"
    lib_name="${lib_name%.dylib}"
    echo "Building for $version_name"
    rm -rf build
    python_include="$(find_dep "$version_name include dir" "$1"/include/)"
    CXXFLAGS=("-I$toolkit_include/ncbi-tools++"
    	      "-I$bpy_include"
    	      "-I$python_include")
    LDFLAGS=("-L$toolkit_lib"
    	     "-L$bpy_lib"
	     "-l$lib_name"
	     "-Wl,-rpath"
	     "-Wl,$toolkit_lib"
	     "-Wl,-rpath"
	     "-Wl,$bpy_lib")
    export _PYTHON_HOST_PLATFORM="macosx-$(sw_vers -productVersion)-$(uname -m)"
    CXXFLAGS="${CXXFLAGS[*]}" LDFLAGS="${LDFLAGS[*]}" \
    	    "$1/bin/$python_bin" -m pip wheel . -w wheelhouse
    echo "Testing package can be imported on $version_name"
    "$1/bin/$python_bin" -m pip install wheelhouse/*-"$version_name"*.whl
    "$1/bin/$python_bin" -c 'import pyblast4_archive'
    "$1/bin/$python_bin" -m pip uninstall -y pyblast4_archive
}

set -e
toolkit="$(brew --prefix ncbi-cxx-toolkit)"
export toolkit_lib="$(find_dep 'NCBI C++ Toolkit library dir' "$toolkit"/lib)"
export toolkit_include="$(find_dep 'NCBI C++ Toolkit include dir' "$toolkit"/include)"
bpy="$(brew --prefix boost-python-cibuildwheel)"
export bpy_lib="$(find_dep 'boost::python library dir' "$bpy"/lib)"
export bpy_include="$(find_dep 'boost::python include dir' "$bpy"/include)"
echo "NCBI C++ Toolkit: $toolkit"
echo "boost::python: $bpy"
set -x
for t in "" "t"; do
    for cp in "/Library/Frameworks/Python${t:u}.framework/Versions/"*; do
	if ! [ -h "$cp" ]; then
	    build "$cp" "cp%s${t}" "python3$t"
	fi
    done
done
# for gp in /Library/Caches/cibuildwheel/graalpy*/; do
#     build "$gp" 'gp%s_*'
# done
for pp in /Library/Caches/cibuildwheel/pypy*/; do
    build "$pp" 'pp%s'
done
