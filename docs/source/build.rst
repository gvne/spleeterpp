============
How To Build
============

Instructions
============

The build system is based on `cmake`.

.. code:: bash

  mkdir build && cd build
  cmake ..
  cmake --build .

The best options for real-time integration would be

OSX
^^^

.. code:: bash

  mkdir build && cd build
  cmake -GXcode -DCMAKE_INSTALL_PREFIX=$(pwd)/install -Dspleeter_enable_tests=OFF -Drtff_use_mkl=ON ..
  cmake --build .


Windows (MSVC2019)
^^^^^^^^^^^^^^^^^^

.. code:: bash

  mkdir build && cd build
  cmake -DCMAKE_INSTALL_PREFIX=$(pwd)/install -Drtff_use_mkl=ON ..
  cmake --build . --config Release --target INSTALL

Options
^^^^^^^

spleeter_enable_tests
^^^^^^^^^^^^^^^^^^^^^

Enable or disable the unit tests. Default is ON

spleeter_enable_high_resolution
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Enable or disable the process extension up the 16KHz. Default is OFF

spleeter_regenerate_models
^^^^^^^^^^^^^^^^^^^^^^^^^^

Uses the `cmake/export_spleeter_models` python scripts to run the conversion
of Spleeter models for c++. Default is OFF

Requires Conda

spleeter_enable_filter
^^^^^^^^^^^^^^^^^^^^^^

Enable the filter interface. Default is ON

spleeter_input_frame_count
^^^^^^^^^^^^^^^^^^^^^^^^^^

Set spleeter `T
<https://github.com/deezer/spleeter/wiki/3.-Models#audio-parameters>`_  parameter

Available options are 64 / 128 / 256 / 512. Default is 64

rtff_use_mkl
^^^^^^^^^^^^

Use the Inter MKL to speed up the Fourier transform and the matrix operations.

Requires a properly install Intel MKL

rtff_use_fftw
^^^^^^^^^^^^^

Use the FFTW for Fourier transform computation.

Only for OSX and Linux.


Tensorflow
==========

The project relies heavily on the `tensorflow` library.
We use the Tensorflow C API for portability. Check the
cmake/add_tensorflow.cmake script for more.
