[![Documentation Status](https://readthedocs.org/projects/spleeterpp/badge/?version=latest)](https://spleeterpp.readthedocs.io/en/latest/?badge=latest)
[![Build Status](https://travis-ci.com/gvne/spleeterpp.svg?branch=master)](https://travis-ci.com/gvne/spleeterpp)
[![Build status](https://ci.appveyor.com/api/projects/status/6sk3vq24ms9oy9l0/branch/master?svg=true)](https://ci.appveyor.com/project/gvne/spleeterpp/branch/master)


# Spleeter C++ Inference

## How ?

[Spleeter](https://github.com/deezer/spleeter) is based on `tensorflow`. As
described in their documentation, we can easily run the inference of such a
model in C++.

## Build

### Note: The system is tested on osx 10.15, ubuntu 18.04 and Windows10 (VS2019)

We base our work on `cmake`.
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

This will:
* Download the `Spleeter` pre-trained models
* Download the pre-built `tensorflow` C API library
* Build the `spleeter++` library and its associated tests

## Documentation

The developer doc is available on readthedoc.  
If you ever want to build it, we provide a dockerfile:
```bash
docker build -t sp-docs -f docs/Dockerfile .
docker run -it --rm -v$(pwd):/code sp-docs doxygen
docker run -it --rm -v$(pwd):/code sp-docs make html
```
