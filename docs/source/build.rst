How To Build
============

Instructions
^^^^^^^^^^^^

The build system is based on `cmake`.

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


<<<<<<< HEAD
Tensorflow
^^^^^^^^^^
=======
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


Tensorflow_cc for Windows
^^^^^^^^^^^^^^^^^^^^^^^^^

To build tensorflow_cc, windows is more complex. First you have to follow this
tutorial: https://www.tensorflow.org/install/source_windows to install
dependencies.

In a few words:

- Download & install this https://www.python.org/ftp/python/3.6.8/python-3.6.8-amd64.exe
- Update your environment variable with C:\Users\[Your User Name]\AppData\Local\Programs\Python\Python36
- download this https://github.com/bazelbuild/bazel/releases/download/0.25.2/bazel-0.25.2-windows-x86_64.exe
- Rename the output as bazel.exe and add its folder to your path (reboot required when updating the path variable)
- Install this https://www.msys2.org/
- In msys2 run pacman -S git patch unzip
- Download this: https://visualstudio.microsoft.com/fr/thank-you-downloading-visual-studio/?sku=Community&rel=15# 
- Launch the install with visual C++ development (top right) selected
- In a powershell run git clone https://github.com/tensorflow/tensorflow.git
- git checkout v1.14.0
- python ./configure.py

When reaching this point, you'll have to edit the exported symbols. Tensorflow
does not include all its symbols because of a dll format limitation that forbid
more that 64K symbols. To do so, open the file
tensorflow/tools/def_file_filter/def_file_filter.py.tpl and edit it to add this:

.. code:: python

  def_fp.write("\t ??0SessionOptions@tensorflow@@QEAA@XZ\n")
  def_fp.write("\t ?LoadSavedModel@tensorflow@@YA?AVStatus@1@AEBUSessionOptions@1@AEBVRunOptions@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEBV?$unordered_set@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@U?$hash@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@U?$equal_to@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@6@QEAUSavedModelBundle@1@@Z\n")
  def_fp.write("\t ??0?$TensorShapeBase@VTensorShape@tensorflow@@@tensorflow@@QEAA@XZ\n")
  def_fp.write("\t ??0?$TensorShapeBase@VTensorShape@tensorflow@@@tensorflow@@QEAA@V?$Span@$$CB_J@absl@@@Z\n")

at line 128. Beware, you need to be consistent in indentation as we are dealing
with python here.

Once done, you can build with:

- bazel build --verbose_failures //tensorflow:tensorflow_cc.dll
- bazel build --verbose_failures //tensorflow:tensorflow_cc.lib

Finally, to get the right headers, the commands are similar to unix system
(see above).

Docker
^^^^^^

To ease and demonstrate the build on linux, a Dockerfile is provided. Building
it will process the library itself and most of its dependencies.

Should you wish to use a pre-built, we uploaded the gvincke/spleeterpp-ci:tf-1.14.0
container to docker hub.

You can also use the pre-built to build a new release. This is used on the CI:

.. code:: bash
>>>>>>> 6dde38e31d8a6ec59fde3bcd897b554e44515a64

The project relies heavily on the `tensorflow` library.
We use the Tensorflow C API for portability. Check the
cmake/add_tensorflow.cmake script for more.
