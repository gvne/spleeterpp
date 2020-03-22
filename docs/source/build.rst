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


Tensorflow
^^^^^^^^^^

The project relies heavily on the `tensorflow` library.
We use the Tensorflow C API for portability. Check the
cmake/add_tensorflow.cmake script for more.
