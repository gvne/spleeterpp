Offline Interface
=================

The simple interface used to run the neural network on large signals.

Initialize
^^^^^^^^^^

.. doxygenfunction:: spleeter::Initialize(const std::string&, const std::unordered_set<SeparationType>&, std::error_code&)

Two stems
^^^^^^^^^^

.. doxygenfunction:: spleeter::Split(const Waveform&, Waveform *, Waveform *, std::error_code&)

Four stems
^^^^^^^^^^

.. doxygenfunction:: spleeter::Split(const Waveform&, Waveform *, Waveform *, Waveform *, Waveform *, std::error_code&)

Five stems
^^^^^^^^^^

.. doxygenfunction:: spleeter::Split(const Waveform&, Waveform *, Waveform *, Waveform *, Waveform *, Waveform *, std::error_code&)
