# Building pyblast4_archive

Before building this library yourself, please consider whether you might be able
to use one of the pre-built [portable binary
wheels](README.md#portable-binary-wheels) instead. You might prefer to instead
build the library if:

* You wish to use a different version of NCBI C++ Toolkit than the one used in
  the pre-built wheels.
* You prefer to use system-installed libraries instead of pre-built shared
  libraries bundled with the pre-built wheels.
* You wish to contribute to `pyblast4_archive` by making changes.
* You wish to customize `pyblast4_archive`.

You might also want to build `pyblast4_archive` if your system is not among the
[platforms](README.md#supported-platforms) supported by the pre-built wheels,
but be advised that unsupported platforms are often unsupported because
`pyblast4_archive` or its dependencies do not build properly on those
platforms. For example, of this writing, neither musl-based Linux (`musllinux`,
e.g. Alpine Linux) nor GraalPy can be used with `pyblast4_archive`.

The portable `pyblast4_archive` wheels are built using the
[`package.yml`](`.github/workflows/package.yml`) workflow. The `package.yml`
YAML file provides precise steps that can be taken to build the portable wheels,
but many these steps are likely unnecessary for a user who simply wants to build
this library for their own system. The instructions in this document should be
easier to follow but do not produce portable wheels.

## Dependencies

`pyblast4_archive` only directly depends on two libraries: NCBI C++ Toolkit, and
Boost.Python. Inconveniently, NCBI C++ Toolkit is in few systems' package
repositories, and although Boost.Python is in some package repositories, the
library must be compiled separately for each Python implementation, so the
system's Boost.Python package might not be useful if you do not also use the
system's Python installation. Hence, instructions for installing both
dependencies from source are provided below.


### Install NCBI C++ Toolkit

#### Dependencies

NCBI C++ Toolkit can be built with many optional dependencies, but the
dependencies of the builds used for the portable wheels are:

* Bzip2
* PCRE
* libgomp (Linux only)
* libzstd
* LMDB (Linux only)
* lzo (macOS only)
* SQLite
* zlib

Additionally, the following dependencies are needed at build time but may be
absent at run time:

* Boost
* Make

As of this writing, Linux builds must be performed with GCC 14 or newer, and
macOS builds must be performed with Apple Clang 16 or newer.

#### Download the C++ toolkit

The latest release of the NCBI C++ Toolkit can be downloaded from its GitHub
repository at https://github.com/ncbi/ncbi-cxx-toolkit-public/releases . To find
the version of the C++ Toolkit used in the portable wheels for the most recent
release of `pyblast4_archive`, consult this repository's [releases
page](https://github.com/actapia/pyblast4_archive/releases) .

#### Build with configure script

The NCBI C++ Toolkit can be built in the same way as most software that uses a
GNU Autotools-based build system.

Despite what some documentation says, you can set a `--prefix` so the software
can be installed wherever you like, but building the bindings will be easier if
you install to a standard location.

```bash
./configure --prefix=/usr/local
```

If configuration succeeds, you can build with `make`. The build process can take
a long time, and you may prefer to run with multiple parallel jobs.

```bash
make
```

Finally, you can install with

```bash
sudo make install
```

#### Build with CMake

Building with the `configure` script can take a long time. CMake can be faster,
but you might be more likely to encounter errors building the C++ Toolkit with
CMake. CMake builds can also be done in parallel, but the C++ Toolkit has
sometimes had problems with dependencies being incorrectly recorded in the
CMakeLists files, so a patch like [this
one](https://github.com/actapia/manylinux_packages/blob/c43c37a8aeb4c4154258eab98f68e25cbbc77b8a/ncbi-cxx-toolkit/rpm/ncbi-cxx-toolkit-parallel-cmake.patch)
might be necessary.

Of course, for this option, CMake must be installed first. Afterwards, run the
`cmake-configure` script, which wraps `cmake`. An install prefix can be proivded
using the `--with-install=` option.

```bash
bash cmake-configure --with-dll --with-install=/usr/local/
```

If `cmake-configure` succeeds, it will have created a directory matching the
glob pattern `CMake*`. The exact name of the created directory reflects the
compiler used and the current build configuration. Change to the `build`
subdirectory of this new directory.

```bash
cd CMake*/build
```

Run `make`.

```bash
make
```

The `install` target of the generated `Makefile` does not lay out the
destination directory structure correctly, so CMake builds must be installed
"manually." If your install destination is at `$DESTDIR`, then run

```bash
sudo install -D ../bin/* -t "$DESTDIR"/bin
sudo install -D ../lib/* -t "$DESTDIR"/lib
sudo mkdir -p "$DESTDIR"/include/ncbi-tools++/
sudo cp -r ../inc/* "$DESTDIR"/include/ncbi-tools++/
cd ../../include
find . -name .svn -prune -o -print |
    sudo cpio -pd "$DESTDIR"/include/ncbi-tools++/
```


### Install Boost.Python

Boost.Python has no dependencies other than the C++ compiler needed to build it
and the Python version for which you want to build the library.

#### Downloading Boost.Python

Boost.Python can be downloaded with the source for the full set of Boost
libraries from the releases page at https://github.com/boostorg/boost/releases .
Boost offers a version that uses CMake and a version that uses `b2` (a.k.a.
Boost.Build or Jam). These instructions assume you've downloaded the version
that uses `b2`.

#### Compiling with b2

For common versions of Python, Boost can automatically detect your Python
location. Alternatively, you can manually add lines to the `project-config.jam`
file generated after running `bootstrap.sh`.

##### Automatic detection

Replace the argument to `--with-python` below with your Python command (e.g.,
`python3` or `/usr/bin/python`).

```bash
./bootstrap.sh "--with-python=python"
```

##### Manual

```bash
./bootstrap.sh
```

After `bootstrap.sh` finishes, add lines based on the following template to
`project-config.jam`.

```text
# Python configuration
import python ;
if ! [ python.configured ]
{
    using python : $VERSION_NUMBER : $PYTHON_BIN : $PYTHON_INCLUDE : $PYTHON_LIB ;
}
```

In the above template, replace the following strings with appropriate values for
the Python for which you want to build Boost.Python.

| Variable          | Value                                                        |
|-------------------|--------------------------------------------------------------|
| `$VERSION_NUMBER` | Python version in `MAJOR.MINOR` format.                      |
| `$PYTHON_BIN`     | Path to the Python executable.                               |
| `$PYTHON_INCLUDE` | Path to the directory containing `Python.h`.                 |
| `$PYTHON_LIB`     | Path to the directory containing `libpython` shared library. |
	

##### Building and installing

To build and install Boost.Python after configuring using the steps above, run

```bash
./b2 install --with-python --prefix=/usr/local
```

You can set the `--prefix` to wherever you want to install Boost.Python. 

If you are building for a Python implementation other than CPython, you might
also want to add a "Python build ID," which is a suffix appended to the library
name. By default, `b2` simply names the library after the Python version
used. To add a build ID, use the `--python-buildid=` parameter.

If you are building for free-threading Python, an additional preprocessor
definition is needed. You can add the required definition by adding an argument
`define=Py_GIL_DISABLED` to the `./b2 install` command.

## Building pyblast4_archive

### Setting environment variables

Before building `pyblast4_archive`, you will need to export some environment
variables for the build process.

#### CXXFLAGS

If the include directories for the target Python version or Boost.Python are
outside the usual locations, you will need to add `-I` flags for them.

You will also likely need to add an `-I` flag for the NCBI C++ Toolkit to add
the `ncbi-tools++` include directory. For example,

```bash
export CXXFLAGS='-I/opt/boost-python/include \
                 -I/opt/python3.12/include/python3.12 \
				 -I/opt/ncbi-cxx-toolkit/include/ncbi-tools++'
```
	
#### LDFLAGS

Since the specific name of the Boost.Python library to be used depends on the
version of Python for which the package is being built, Boost.Python is omitted
from the list of libraries to link against in the `pyproject.toml` for
`pyblast4_archive`. Hence, a flag will need to be added to `LDFLAGS` to tell the
linker to link against the specific Boost.Python library you would like to
use. For example, if you are building for CPython 3.12 and have not appended a
build ID to the library, you would add `-lboost_python312` to the `LDFLAGS`. On
some systems, `--no-as-needed` may also be necessary to ensure the linker 
uses the `-lboost_python` argument.

If the libraries for Boost.Python, or the NCBI C++ toolkit are outside of the
usual locations searched by the compile-time linker, you will need to add `-L`
flags for them.

Likewise, if either of those libraries is outside of the usual locations
searched by the runtime linker, you will need to add `-Wl,rpath` options for
them. For example,

```bash
export LDFLAGS='-Wl,--no-as-needed \
                -L/opt/boost-python/lib \
                -L/opt/ncbi-cxx-toolkit/lib \
				-Wl,-rpath \
				-Wl,/opt/boost-python/lib \
				-Wl,-rpath \
				-Wl,/opt/ncbi-cxx-toolkit/lib \
				-lboost_python312'
```

### Building and installing

Once you've set all the relevant environment variables, you can build from the
root of this repository with

```bash
python -m pip wheel . -w wheelhouse
```

If the command succeeds, the built wheel will be in the `wheelhouse`
directory. You can install it using `pip install`.

Alternatively, you can build and install from source fetched from PyPI with

```bash
python -m pip install --no-binary pyblast4_archive
```

