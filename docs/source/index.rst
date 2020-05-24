Welcome to the Spleeterpp Library documentation!
================================================

*Spleeterpp* is a library to ease the use of the `Spleeter
<https://github.com/deezer/spleeter>`_ project in C++ programs.

What it is
^^^^^^^^^^

This project is an attempt at bridging the gap between the spleeter research
team and people looking to integrate their work in creative ways.

What it is NOT
^^^^^^^^^^^^^^

Spleeter only runs on 44.1KHz stereo input. We do not provide any means to
convert a signal in that format as it is already covered by other tools.

Example
^^^^^^^

The following snippet shows how to use the two stems extraction.

.. code:: cpp

  #include "spleeter/spleeter.h"

  ...

  // Initialize spleeter
  spleeter::Initialize(std::string(SPLEETER_MODELS), {spleeter::TwoStems}, err);

  // Read the input data, convert it to 44.1KHz and Map it into a Waveform
  ...

  // Run the two stems extraction
  spleeter::Waveform vocals, accompaniment;
  spleeter::Split(source, &vocals, &accompaniment, err);


.. toctree::
   :maxdepth: 2
   :caption: Contents:

   build
   offline
   filter
