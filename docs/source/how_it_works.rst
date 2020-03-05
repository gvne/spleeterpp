How it works
============

Tensorflow model
^^^^^^^^^^^^^^^^

*Spleeter* is designed to work on raw audio file. The provided neural netwok
inputs a stereo signal sampled at 44.1KHz and input multiple signals in a
similar format.

To run it on-line, we export a cropped version of that network that uses STFT
(Short Time Fourier Transform) frames and outputs Masks (matricies of values in
[0, 1] that needs to be multiplies to the STFT to get the output source). To do
so, we designed the `cmake/export_spleeter_models.py` script.

Model parameter
^^^^^^^^^^^^^^

The provided models have several parameters. Specifically, the *T* parameter is
crucial for us. It represents the time length of the input spectrogram segment
expressed in short time Fourier transform frames. Meaning that every time we run
the extraction, we will run it on T frames. Then it is easy to understand that
the larger it is, the better the extraction but the larger the latency will be.

As this parameter may vary from a project to another, we provide a way to update
it. See the Build section for more.

Filter parameter
^^^^^^^^^^^^^^^^

The basic idea is:
* We run each process on P frames (P <= T)
* We effectively use F frames in each process (F <= P)
* The F frames are centered in the processed buffer. The rest of the processed buffer is filled with past and future data
* We use an overlap of O frames between each process (0 <= O < F). In the overlap, we mean the output
* In Spleeter, the sum of extracted signals may differ from the input. We offer a way to force it using the force conservativity parameter
