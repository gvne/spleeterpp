Welcome to the Spleeterpp Library documentation!
================================================

*Spleeterpp* is a library to ease the use of the `Spleeter
<https://github.com/deezer/spleeter>`_ project in C++ programs.

What it convers
^^^^^^^^^^^^^^^

This project is an attempt at bringing audio source separation into a real-time
process.


What it does NOT cover
^^^^^^^^^^^^^^^^^^^^^^

Spleeter only runs on 44.1KHz stereo input. We do not provide any means to
convert a signal in that format as it is already covered by other tools.

Example
^^^^^^^

The following snippet shows how to use the two stems extraction.

.. code:: cpp

  #include "spleeter/spleeter.h"

  ...

  const auto separation_type = spleeter::FourStems;

  // Initialize spleeter
  spleeter::Initialize(std::string(SPLEETER_MODELS), {separation_type}, err);

  // Initialize the on-line filter
  spleeter::Filter filter(separation_type);
  filter.Init(err);

  // initialize a buffer
  const auto block_size = 2048;  // defined by the plug-in host
  filter.set_block_size(block_size);
  rtff::AudioBuffer buffer(block_size, filter.channel_count());

  ...

  // For each block
  float* data_ptr = ...
  buffer.fromInterleaved(sample_ptr);
  filter.ProcessBlock(&buffer);
  buffer.toInterleaved(sample_ptr);


.. toctree::
   :maxdepth: 2
   :caption: Contents:

   how_it_works
   build
   reference
