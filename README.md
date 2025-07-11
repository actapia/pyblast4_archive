# pyblast4_archive

These are *very* incomplete Python bindings to the CBlast4_archive objects from
the [NCBI C++ Toolkit](https://github.com/ncbi/ncbi-cxx-toolkit-public). I might
add more to them later if the need arises.

## Dependencies

This software has been tested with the following dependencies. Similar
configurations might also work.

* NCBI C++ Toolkit 29.3.0
* Boost.Python 1.83.0

## Installing NCBI C++ Toolkit

NCBI C++ toolkit can be tricky to build and is not in the default repositories
of Debian or Ubuntu as of this writing, so I provide some basic instructions for
building the C++ toolkit itself here.

To build the relevant parts of the NCBI C++ Toolkit with GCC on Linux, you'll
need GCC 14 or newer. For builds on macOS, you should be able to use a recent
version of Clang (e.g., Apple Clang 16.0.0 or newer). As of this writing,
building with Clang on Linux doesn't seem to be working.

You also probably need to install `libbz2`, `cmake`, `zlib`, and various `boost`
libraries (get all of them, if you can).

Download the source tarball from the GitHub releases, extract the archive and
change to the root directory of the repository. 
<!-- We only want to build part of -->
<!-- the toolkit, so we'll create a file called `projects` that specifies which parts -->
<!-- to build. The `projects` file should have the following contents: -->

<!-- ```text -->
<!-- corelib$ -->
<!-- util -->
<!-- objects -->
<!-- -objects/.*/test -->
<!-- -objects/[^/]*/demo -->
<!-- -objects/genomecoll/gc_cli -->
<!-- serial -->
<!-- -serial/test -->
<!-- test update-only -->
<!-- ``` -->

<!-- Then, configure your build with `./configure`: -->

<!-- ```bash -->
<!-- ./configure --with-projects=projects -->
<!-- ``` -->

Configure your build with `./configure`:

```bash
./configure
```

If that succeeds, build with `make`:

```bash
make
```

then install

```bash
sudo make install
```
