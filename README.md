# pyblast4_archive

This library provides limited Python bindings to the 
[CBlast4_archive](https://www.ncbi.nlm.nih.gov/IEB/ToolBox/CPP_DOC/doxyhtml/classCBlast4__archive.html)
and related objects from the 
[NCBI C++
Toolkit](https://github.com/ncbi/ncbi-cxx-toolkit-public). CBlast4_archive
allows the programmer to interpret BLAST output in format 11 "BLAST archive
format (ASN.1)," and this Python library aims to make such interpretation
possible in Python.

## Installation

The latest [portable binary wheel](#portable-binary-wheels) for your platform
can be installed from PyPI using `pip`.

```python
python -m pip install pyblast4_archive
```

Alternatively, you can [build the library yourself](./BUILDING.md).

## Getting started

The main class of interest in this library is `Blast4Archive`. To load 
`Blast4Archive` objects from a binary ASN.1 BLAST Archive file, use the
`from_path` `classmethod`.

```python
fomr pyblast4_archive import Blast4Archive

archives = Blast4Archive.from_path("blast_results.asn1", "asn_binary")
```

or for a text ASN.1 file,

```python
archives = Blast4Archive.from_path("blast_results.asn1", "asn_text")
```

To decode query IDs from the archives, use `decode_query_ids`.

```python
from pyblast4_archive import decode_query_ids

decoded_queries = decode_query_ids(archives)
```

Likewise, to decode subejct IDs, use `decode_subject_ids`.

```python
from pyblast4_archive import decode_subject_ids

decoded_subjects = decode_subject_ids(archives)
```

To decode BLAST database OIDs, a `SeqDB` object is needed. `SeqDB` objects can
be constructed from a path to the BLAST DB and its database type (`"nucleotide"`
or `"protein"`).

```python
from pyblast4_archive import SeqDB

db = SeqDB("/path/to/blast/db", "nucleotide")
```

Then, to decode database OIDs, use `decode_database_oids`.

```python
from pyblast4_archive import decode_database_oids

decoded_oids = decode_database_oids(archives, db)
```

## Dependencies

This software depends on the NCBI C++ Toolkit and Boost.Python. 

## Portable binary wheels

Since this software provides Python bindings for a C++ library, it must be
compiled, unlike a pure Python library. Pre-built portable binary wheels for
many platforms are avaiable on the Python Package Index (PyPI). The full process
for building these wheels is described by the
[`package.yml`](.github/workflows/package.yml) GitHub Actions workflow file.

### Supported platforms

The platforms for which I am currently building portable wheels to upload to
PyPI are listed below. For other platforms, it might be necessary to [build this
library yourself](./BUILDING.md).

#### Linux, glibc (manylinux)

##### Architectures

* x86_64
* i686
* aarch64
* s390x
* ppc64le

##### Python implementations

* CPython 3.11
* CPython 3.12
* CPython 3.13 (+ free threading)
* CPython 3.14 (+ free threading)
* PyPy 3.11

#### macOS

##### Architectures

* x86_64
* aarch64

##### Python implementations

* CPython 3.11
* CPython 3.12
* CPython 3.13 (+ free threading)
* CPython 3.14 (+ free threading)

##### macOS versions

* macOS 15 (Sequoia)
* macOS 26 (Tahoe)

### Dependency licenses

Portable binary wheels operate by bundling the main library with its transitive
dependencies. This is accomplished with
[auditwheel](https://github.com/pypa/auditwheel) on Linux or
[delocate](https://github.com/matthew-brett/delocate) on macOS and is abstractly
similar to static linking. Source code for these transitive dependencies are
*not*  included in the Git repository for this library since those wanting to
build this library themselves will most likely prefer to download the transitive
dependencies from a package manager or else build them from upstream sources.

The sources for the transitive dependencies *are* included in the source
tarballs accompanying each release of this software on
[GitHub](https://github.com/actapia/pyblast4_archive/releases). (Due to file size
limitations, the source tarballs available from PyPI *do not* currently include
these dependencies.) Dependency sources for Linux can be found under the
`deps/linux` directory, and dependency sources for macOS can be found under the
`deps/macos` directory.

Both the binaries and sources of the transitive dependencies are obtained from
the package manager of the system used for building. For Linux, the
`manylinux_2_28` container (based on AlmaLinux 8) was used. The `manylinux_2_28`
container uses the default AlmaLinux 8 repositories, Extra Packages for
Enterprise Linux, and a custom DNF repository hosted at
https://www.cs.uky.edu/~acta225/manylinux_packages/almalinux/8 . For macOS, the
default Homebrew taps `hombrew/core` and `homebrew/cask` were used, as well as a
custom `actapia/cibuildwheel_packages` tap.

For Linux dependencies, the RPM `.spec` file and any patches associated with the
package are included. For macOS dependencies, the Homebrew Formula `.rb` file is
included. In both cases, any patches for the pakcage have been pre-applied.

Full license texts for each dependency may be found with the dependency's source
code under `deps/`. Additionally, the [LICENSES.linux](LICENSES.linux) and
[LICENSES.macos](LICENSES.macos) files summarize the licenses of the transitive
dependencies bundled in the portable wheels.

## License

This software is licensed under the GNU General Public License, Version 3, which
may be found at [LICENSE](LICENSE).
