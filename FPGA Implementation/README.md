# FPGA Implementation

This is the implementation for Swing Filter.

## Platform

This implementation is completed on a NetFPGA-1G-CML FPGA development board with 4.5 MB SRAM and four Ethernet interfaces capable of negotiating up to 1 GB/s connections. The development board is connected to a workstation (with a Ryzen7 1700 @3.0GHZ CPU and 64 GB RAM) through PCIe Gen2 X4 lanes. 

## Description

We provide C++ codes for fpga implementation here, which can be directly synthesized into Verilog codes using Vivado HLS. In addition, we also provide synthesized Verilog code in the *./Verilog* directory to facilitate readers' reproduction.
