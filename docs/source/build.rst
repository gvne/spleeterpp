How To Build
============

Instructions
^^^^^^^^^^^^

The build system is based on `cmake`. It has two external dependencies:

- `conda` has to be found in the path
- `tensorflow_cc` install path should be provided

Then you can build it like any other `cmake` project.

.. code:: bash

  mkdir build && cd build
  cmake -DTENSORFLOW_CC_INSTALL_DIR=/my/install/dir ..
  cmake --build .


Tensorflow_cc
^^^^^^^^^^^^^

The project relies heavily on the `tensorflow_cc` library.
To build it you should start by cloning it:


.. code:: bash

  git clone https://github.com/tensorflow/tensorflow.git
  cd tensorflow
  git checkout v1.14.0

Then, install the dependencies and configure:

.. code:: bash

  rm BUILD  # problem with setup.py os osx
  python tensorflow/tools/pip_package/setup.py install
  mv build build-bu
  git checkout BUILD
  ./configure

Build the library (it will take a while...):

.. code:: bash

  bazel build --config=monolithic --jobs=6 --verbose_failures //tensorflow:libtensorflow_cc.so

Finally, install everything in a specific folder:

.. code:: bash

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
