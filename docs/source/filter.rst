Filter Interface
================

The filter interface lets you process the signal on-line. This is designed for
real-time intergration.


Snippet
^^^^^^^

.. code:: cpp

  #include "spleeter_filter/filter.h"

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


Reference
^^^^^^^^^

.. doxygenfunction:: spleeter::Initialize(const std::string&, const std::unordered_set<SeparationType>&, std::error_code&)

.. doxygenclass:: spleeter::Filter

Tensorflow model
^^^^^^^^^^^^^^^^

*Spleeter* is designed to work on raw audio file. The provided neural netwok
inputs a stereo signal sampled at 44.1KHz and outputs multiple signals in a
similar format.

To run it on-line, we export a cropped version of that network that inputs STFT
(Short Time Fourier Transform) frames and outputs Masks (matricies of values in
[0, 1] that needs to be multiplies to the STFT to get the output source). To do
so, we designed the `cmake/export_spleeter_models.py` script.

Data Size
^^^^^^^^^

To understand how the filter runs, it is important to understand the tensor sizes

.. image:: https://gvne-public.s3.amazonaws.com/spleeterpp/Spleeterpp+-+Filter+parameter+def.jpg


Flow Description
^^^^^^^^^^^^^^^^

The basic idea is:

- We run each process on P frames (P <= T)
- We effectively use F frames in each process (F <= P)
- The F frames are centered in the processed buffer. The rest of the processed buffer is filled with past and future data
- We use an overlap of O frames between each process (0 <= O < F). In the overlap, we mean the output
- In Spleeter, the sum of extracted signals may differ from the input. We offer a way to force it using the force conservativity parameter


.. image:: https://gvne-public.s3.amazonaws.com/spleeterpp/Spleeterpp+-+Flow.jpg
