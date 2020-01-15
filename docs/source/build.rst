How To Build
============

Instructions
^^^^^^^^^^^^

The build system is based on `cmake`. It requires `conda` installed and
available on your path.

Then you can build it like any other `cmake` project.

.. code:: bash

  mkdir build && cd build
  cmake -DTENSORFLOW_CC_INSTALL_DIR=/my/install/dir ..
  cmake --build .


Tensorflow_cc
^^^^^^^^^^^^^

The project relies heavily on the `tensorflow_cc` library.
We provide pre-built version but you can also use your version using the
TENSORFLOW_CC_INSTALL_DIR cmake variable.
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


Docker
^^^^^^

To ease and demonstrate the build on linux, a Dockerfile is provided. Building
it will process the library itself and most of its dependencies.

Should you wish to use a pre-built, we uploaded the gvincke/spleeterpp-ci:tf-1.14.0
container to docker hub.

You can also use the pre-built to build a new release. This is used on the CI:

.. code:: bash

  echo "mkdir -p /code/build" >  build.sh
  echo "cd /code/build" >> build.sh
  echo "cmake -DTENSORFLOW_CC_INSTALL_DIR=/spleeterpp/tensorflow/install .." >> build.sh
  echo "cmake --build ." >> build.sh
  docker run -v$(pwd):/code gvincke/spleeterpp-ci:tf-1.14.0 bash /code/build.sh
