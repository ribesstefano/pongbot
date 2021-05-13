# Playing Pong on HDMI Screen

## Software

## Hardware

### CMake Simulation

#### Windows

Simulation is working assuming the DLL are copied in `bin/` along side the executable.
```
"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
mkdir build
cmake .. -G Ninja
cmake --build . --config Release
```

#### Linux
```
mkdir build
cd build
cmake ..
make all
```
