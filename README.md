# Swing Filter

---

This repository contains all related codes of our paper "Swing Filter: A Low-overhead Filter with Larger Filtering Range for Network Traffic Measurement".

## Introduction

Traffic measurement provides crucial statistics for network management, serving as the foundation for various applications such as quality of service, network billing,
load balancing, and anomaly detection. However, the scarcity of resources on network devices restricts measurement solutions to providing only approximate results. Meanwhile, highly skewed distribution of network traffic further compromise the measurement accuracy. Although filtering the vast majority of small flows in advance can help to improve the estimation performance, the existing filters have limitations in filtering range and processing overhead. This paper proposes an efficient filter with a flexible and extended filtering range for network traffic measurement. One key to the design is the use of signed counters whose values swing in positive and negative directions to cancel out small flows, thereby enlarging the filtering range. We show that the proposed filter is highly effective in filtering small flows, with lower memory overhead and processing overhead than the existing work. It supports various measurement tasks and provides a guaranteed bound on misreport rate. We implement our filter on P4 and a NetFPGA-equipped prototype, and conduct extensive experiments based on real-world Internet traces collected by CAIDA. Experimental results show that the proposed filter reduces the flow-size estimation error by an order of magnitude and achieves 1.69 times higher throughput than SOTA.

## About this repo

- `data` contains a sample of CAIDA 2019 IP traces.
- `CPU Implementation` contains source code for C++ and Python implementations of Swing Filter and related work. The Python version includes tests for typical measurement tasks: flow size estimation, top-k query, and heavy hitter detection.
- `FPGA Implementation` contains the source code for NetFPGA implementation.
- `P4 Implementation` contains the source code for the implementation on P4 switches.
