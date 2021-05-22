# Playing Pong Against AI Bot

## Overview

This project includes the game of Pong running on a Digilent PYNQ-Z1 FPGA board.
The player is challenged to play against a bot trained with Deep-Q Reinforcement Learning (DQRL).

### Software/AI

The bot AI is trained in Google Colab following the notebook in `ai/` folder.

### Hardware

The hardware part was implemented in Vivado High Level Synthesis (HLS) C++. It exploits Xilinx OpenCV libraries for resizing and stream the video frames generated by the game.

#### Pong Game Cosimulation

#### OpenCV Cosimulation

#### HDMI Modules

For instantiating the HDMI IPs, please follow this [guide](https://forums.xilinx.com/t5/Design-and-Debug-Techniques-Blog/Video-Series-23-Generate-a-video-output-on-Pynq-Z2-HDMI-out/ba-p/932553).

#### Vivado Project

## Requirements

* CMake
* Xilinx Vivado 2018.3

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

## Bugs

List of possible bugs:
* Having not squared images in games generates distorted images.