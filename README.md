# UnNamedAmpSim
UnNamed Amp Sim (UNAS) is a Windows guitar VST for ASIO audio interfaces.

![Screenshot 2023-09-09 152004](https://github.com/KDOsborne/UnNamedAmpSim/assets/34141764/36a39799-e901-4e0f-8620-27b41bfbab5b)

Your audio interface should be set to 48000Hz sample rate with a buffer size less than 256 samples to minimize monitoring delay. 
 
 Current features include:
  + 10-band EQ
  + Asymmetrical distortion with customizable clipping algorithms
  + Live tuning


UNAS relies on the following dependencies. I'm using the 64-bit pre-built versions of each of these libraries. If you want to build them yourself, note that BASSASIO may give you trouble due to a lack of ASIO source code:
  + BASS
  + BASS_FX
  + BASSASIO
  + libaubio

To build, simply run make.
