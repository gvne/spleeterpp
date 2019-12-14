[![Documentation Status](https://readthedocs.org/projects/spleeterpp/badge/?version=latest)](https://spleeterpp.readthedocs.io/en/latest/?badge=latest)

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
cmake -DTENSORFLOW_CC_INSTALL_DIR=/my/install/dir ..
cmake --build .
```

This will:
* Clone `Spleeter` and checkout its release version.
* Create the `conda` environment
* Download the pre-trained weights
* Generate each models
* Build the `spleeter++` library and its associated tests

## Help

### Build `tensorflow_cc`

We need to match the version used in `Spleeter` i.e. *1.14.0*.

The build system is based on `Bazel`. Install the right (*0.25.2*) version found [here](https://github.com/bazelbuild/bazel/releases).

Then, clone the repository and checkout the right tag:
```bash
git clone https://github.com/tensorflow/tensorflow.git
cd tensorflow
git checkout v1.14.0  # select the version you want...
```

Install dependencies and configure:
```bash
rm BUILD  # problem with setup.py os osx
python tensorflow/tools/pip_package/setup.py install
mv build build-bu
git checkout BUILD
./configure
```

Then, build the library (it will take a while...):
```bash
bazel build --config=monolithic --jobs=6 --verbose_failures //tensorflow:libtensorflow_cc.so
```

Finally, install everything in a specific folder:
```bash
INSTALL_DIR=install
INCLUDE_DIR=$INSTALL_DIR/include

# libraries
mkdir -p $INSTALL_DIR/bin
cp bazel-bin/tensorflow/libtensorflow_cc.so* $INSTALL_DIR/bin/

# and headers
mkdir -p $INSTALL_DIR/include
rsync -a --prune-empty-dirs --include '*/' --include '*.h' --exclude '*' tensorflow/ $INCLUDE_DIR/tensorflow
mkdir -p $INSTALL_DIR/include/third_party/eigen3/unsupported/
cp -r ./bazel-tensorflow/external/eigen_archive/unsupported/Eigen $INSTALL_DIR/include/third_party/eigen3/unsupported/Eigen
cp -r ./bazel-tensorflow/external/eigen_archive/Eigen $INSTALL_DIR/include/third_party/eigen3/Eigen
```
