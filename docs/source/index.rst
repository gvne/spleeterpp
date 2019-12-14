Welcome to the Spleeterpp Library documentation!
================================================

*Spleeterpp* is a library to ease the use of the `Spleeter
<https://github.com/deezer/spleeter>`_. project in C++ programs.

Example
^^^^^^^

The following snippet shows how to use the two stems extraction.

.. code:: cpp

  #include "spleeter/spleeter.h"

  ...

  float* input_data = ...
  uint64_t sample_count = ...
  const uint8_t channel_count = 2;  // Only stereo supported
  auto input = Eigen::Map<spleeter::Waveform>(
    input_data, channel_count, sample_count/ channel_count);

  std::error_code err;
  spleeter::Initialize("path/to/saved/models", {spleeter::TwoStems}, err);
  if (err) {
    std::cerr << "Initialization failed" << std::endl;
    ...
  }

  spleeter::Waveform vocals, accompaniment;
  spleeter::Split(input, &vocals, &accompaniment, err);
  if (err) {
    std::cerr << "Something went wrong..." << std::endl;
    ...
  }


.. toctree::
   :maxdepth: 2
   :caption: Contents:

   build
   reference
