# UnNamedAmpSim
UnNamed Amp Sim (UNAS) is a Windows guitar VST for ASIO audio interfaces.

![Screenshot 2023-09-09 155543](https://github.com/KDOsborne/UnNamedAmpSim/assets/34141764/5fdc5589-148e-4393-b5b1-8651d86cb0a1)

Your audio interface should be set to 48000Hz sample rate with a buffer size less than 256 samples to minimize monitoring delay. 
 
 Current features include:
  + 10-band EQ
  + Asymmetrical distortion with customizable clipping algorithms
  + Live tuning


UNAS relies on the following dependencies:
  + BASS
  + BASS_FX
  + BASSASIO
  + GLAD
  + libaubio

I'm using the 64-bit pre-built versions of each of these libraries. If you want to build them yourself, note that BASSASIO may give you trouble due to unavailablility of ASIO C source code

To build, simply run make.
