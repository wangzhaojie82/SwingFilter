import concurrent
import re
import Packet
import mmh3
import numpy as np
from sketches import CountMin

class LLF:
    def __init__(self, m, skt):
        self.m = m
        self.r = 3 # number of hash
        self.f = 0
        self.sketch = skt
        self.delta = 10 # the threshold
        self.phi = 0.77351
        self.R = np.zeros(shape=(self.m,), dtype=np.int8)
        self.seeds = [np.random.randint(i * 10000, (i + 1) * 10000) for i in range(self.r)]
        self.real_size = {}
        self.pred_size = {}

    def get_leftmost(self, random_val):
        left_most = 0
        while random_val:
            left_most += 1
            random_val >>= 1
        return 32 - left_most

    def update(self, packet):
        src = packet.flow_label
        if src in self.real_size.keys():
            self.real_size[src] += 1
        else:
            self.real_size[src] = 1

        gamma = 0xffffffff
        for i in range(self.r):
            idx = mmh3.hash(src, seed=self.seeds[i], signed=False) % self.m
            gamma = min(self.R[idx], gamma)
        if gamma < self.delta:
            self.f += 1
            for i in range(self.r):
                idx = mmh3.hash(src, seed=self.seeds[i], signed=False) % self.m
                random_val = np.random.randint(0, 2147483647)
                leftmost = self.get_leftmost(random_val)
                self.R[idx] = max(min(leftmost, self.delta), self.R[idx])
        else:
            self.sketch.update(packet)

    def report(self, src):
        filter_est = 0.0
        reg_sum = 0.0
        sketch_est = 0.0
        gamma = 0xffffffff
        for i in range(self.r):
            idx = mmh3.hash(src, seed=self.seeds[i], signed=False) % self.m
            gamma = min(gamma, self.R[idx])
            reg_sum += self.R[idx]
        reg_sum = 2 ** (reg_sum // self.r)
        filter_est = (self.m * self.r) / (self.m - self.r) * (1 / (self.r * self.phi) * reg_sum - self.f / self.m)
        if gamma == self.delta:
            sketch_est = self.sketch.report(src)
        return int(round(filter_est + sketch_est))

    def calculate_are(self):
        total_flows = 0
        total_relative_error = 0

        for src, size in self.real_size.items():
            estimated_size = self.report(src)
            if size > 0:
                relative_error = abs(size - estimated_size) / size
                total_relative_error += relative_error
                total_flows += 1

        are = round(total_relative_error/total_flows, 3)
        return are





if __name__ == '__main__':


    # the path for your dataset
    traffic_traces = ['./data/your_dataset.txt']

    filename = f'./your_filename.txt'
    with open(filename, 'w') as file:

        memory_frac_of_filter = 0.33

        # memory setting, in KB
        for memory_kb in [100, 200]:

            memory_kb_Filter = round(memory_kb * memory_frac_of_filter)
            memory_kb_Sketch = memory_kb - memory_kb_Filter

            sketch = CountMin.init_CountMin(memory_kb_Sketch)

            m = (memory_kb_Filter * 1024 * 8) // 4  # number of registers

            llf = LLF(m, sketch)

            total_packets = 0
            pattern = r"\b(?:\d{1,3}\.){3}\d{1,3}\b"
            for traffic_trace in traffic_traces:
                with open(traffic_trace, 'r', encoding='utf-8') as f:
                    for line in f:
                        tmp = line.split()
                        # filter the invalid IPv4 packets
                        if not re.match(pattern, tmp[0]) or not re.match(pattern, tmp[1]):
                            continue
                        packet = Packet.Packet(total_packets, tmp[0], tmp[1])

                        llf.update(packet)

            ARE = llf.calculate_are()

            file.write(f"Total Memory = {memory_kb} |  ARE = {ARE}")
            file.write(f"\n")

            print(f"Total Memory = {memory_kb} |  ARE = {ARE}")





