[![Documentation Status](https://readthedocs.org/projects/spleeterpp/badge/?version=latest)](https://spleeterpp.readthedocs.io/en/latest/?badge=latest)
[![Build Status](https://travis-ci.com/gvne/spleeterpp.svg?branch=master)](https://travis-ci.com/gvne/spleeterpp)

# Spleeter C++ Inference

## Documentation

The developer doc is available on readthedoc.  
If you ever want to build it, we provide a dockerfile:
```bash
docker build -t sp-docs -f docs/Dockerfile .
docker run -it --rm -v$(pwd):/code sp-docs doxygen
docker run -it --rm -v$(pwd):/code sp-docs make html
```

## How ?

[Spleeter](https://github.com/deezer/spleeter) is based on `tensorflow`. As
described in their documentation, we can easily run the inference of such a model in C++.

## Requirements

* `conda` installed and available in your path
* the `tensorflow_cc` library (see the Help section for instructions)

## Build

We base our work on `cmake`.
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

This will:
* Clone `Spleeter` and checkout its release version.
* Create the `conda` environment
* Download the pre-trained weights
* Generate each models
* Download the `tensorflow_cc` library
* Build the `spleeter++` library and its associated tests
