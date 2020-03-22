[![Documentation Status](https://readthedocs.org/projects/spleeterpp/badge/?version=latest)](https://spleeterpp.readthedocs.io/en/latest/?badge=latest)
[![Build Status](https://travis-ci.com/gvne/spleeterpp.svg?branch=master)](https://travis-ci.com/gvne/spleeterpp)

# Spleeter C++ Inference

## How ?

[Spleeter](https://github.com/deezer/spleeter) is based on `tensorflow`. As
described in their documentation, we can easily run the inference of such a
model in C++.

## Build

### Note: The system is tested on osx 10.15, ubuntu 18.04 and Windows10 (VS2017)

We base our work on `cmake`.
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

For windows, we only support Visual Studio 2017 in Release mode (see [this](https://github.com/tensorflow/tensorflow/issues/17778#issuecomment-384816014)).
```bash
mkdir build && cd build
cmake -G"Visual Studio 15 2017 Win64"  ..
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
