[![Documentation Status](https://readthedocs.org/projects/spleeterpp/badge/?version=latest)](https://spleeterpp.readthedocs.io/en/latest/?badge=latest)
[![Build Status](https://travis-ci.com/gvne/spleeterpp.svg?branch=master)](https://travis-ci.com/gvne/spleeterpp)

# Spleeter C++ Inference

## How ?

[Spleeter](https://github.com/deezer/spleeter) is based on `tensorflow`. As
described in their documentation, we can easily run the inference of such a
model in C++.

## Build

### Note: We only support Unix system at the moment (tested on osx 10.15 and ubuntu 18.04)

We base our work on `cmake`.
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

This will:
* Download `Spleeter` the pre-trained weights
* Download the pre-built `tensorflow_cc` library
* Build the `spleeter++` library and its associated tests

## Documentation

The developer doc is available on readthedoc.  
If you ever want to build it, we provide a dockerfile:
```bash
docker build -t sp-docs -f docs/Dockerfile .
docker run -it --rm -v$(pwd):/code sp-docs doxygen
docker run -it --rm -v$(pwd):/code sp-docs make html
```
