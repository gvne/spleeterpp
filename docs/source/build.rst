How To Build
============

Instructions
^^^^^^^^^^^^

The build system is based on `cmake`. It requires `conda` installed and
available on your path.

Then you can build it like any other `cmake` project.

.. code:: bash

  mkdir build && cd build
  cmake ..
  cmake --build .


Spleeter pre-trained models
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The original Spleeter project provide pre-trained model using a tensorflow
python specific format.

We provided the same models in a converted format in our repository. The cmake
script will take care of the download step for you.

For those who want to re-generate them from Spleeter, we provide a cmake option
(spleeter_regenerate_models) that does it for you. Beware that this requires
`conda` installed and available in your path. It will clone the original
repository, build the conda environment and run a export script
(cmake/export_spleeter_models.py).


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
  cp bazel-bin/tensorflow/libtensorflow_cc.so.1.14.0 $INSTALL_DIR/bin/libtensorflow_cc.so.1
  ln -s $INSTALL_DIR/bin/libtensorflow_cc.so.1 $INSTALL_DIR/bin/libtensorflow_cc.so

  # and headers
  mkdir -p $INCLUDE_DIR
  rsync -a --prune-empty-dirs --include '*/' --include '*.h' --exclude '*' tensorflow/ $INCLUDE_DIR/tensorflow
  rsync -a --prune-empty-dirs --include '*/' --include '*.h' --exclude '*' bazel-bin/tensorflow/ $INCLUDE_DIR/tensorflow

  mkdir -p $INCLUDE_DIR/third_party/eigen3/unsupported/
  cp -r ./third_party/eigen3/unsupported/Eigen $INCLUDE_DIR/third_party/eigen3/unsupported/Eigen


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
