# Buddy Python SWIG Bindings

The SWIG python bindings are built use the automated CMake build system or manual instructions
are provided for reference.  The binding is built as part of the global project build also.

## Preface

The installation script uses a bit of a hack to grab the `--libpath` and `--swigpath` arguments.  The
`libpath` specifies the file path to the library directory.  The `swigpath` argument specifies the file
path to the generated C swig binding header and source files.  

## Automated

From within the global project root or the `bindings/python` directory.

```shell
mkdir -p build
cmake ..
make
```

## Manual

```shell
swig -outcurrentdir -I../../../libbuddy/common -I../../../libbuddy/host -python buddy.i
python setup.py --libpath ../../.. --swigpath . build install
```