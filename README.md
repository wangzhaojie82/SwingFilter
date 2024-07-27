# Swing Filter

---

This repository contains all related codes of our paper "Swing Filter".

## Introduction

Traffic measurement provides crucial statistics for network management, serving as the foundation for various applications such as quality of service, network billing,
load balancing, and anomaly detection. However, the scarcity of resources on network devices restricts measurement solutions to providing only approximate results. Meanwhile, high-proportion small flows in network traffic further compromise the measurement accuracy. In this paper, we design Swing Filter to accelerate traffic measurement and improve accuracy. Swing Filter utilizes reversible counters to capture small flows, enhancing the filtering capability. Moreover, it relies on one-direction communication, and each packet only updates one counter without costly operations. Swing Filter can be applied to various measurement tasks, and improves measurement accuracy. Experiments on CAIDA IP traces show improvements achieved by Swing Filter in measurement tasks. In flow size estimation, Swing Filter reduces the estimation error by three orders of magnitude and achieves 49× higher throughput than the seminal scheme. Swing Filter is implemented on software and hardware.

## About this repo

- `data` contains a sample of CAIDA 2016 IP traces.
- `CPU Implementation` contains source code for C++ and Python implementations of Swing Filter and related work. The Python version includes tests for four measurement tasks: flow size estimation, top-k query, and heavy hitter/change detection.
- `FPGA Implementation` contains the source code for NetFPGA implementation.
- `P4 Implementation` contains the source code for the implementation on P4 switches.
