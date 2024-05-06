
# The implementation of CountMin + min-heap to query top-k flows

import mmh3
import heapq
import numpy as np

from sketches import CountMin


class CMHeap:
    def __init__(self, count_min):
        self.count_min = count_min
        self.heap = []
        self.flow_set = set()  # Use set to record the flow label inserted


    def update(self, packet):
        # update count-min
        self.count_min.update(packet)

        self.flow_set.add(packet.flow_label)  # Use add method to add flow label to set


    def query(self, k):
        """
        :return top k flows and their size. A list of tuple (flow_label, size)
        """
        for flow_label in self.flow_set:
            estimated_size = self.count_min.report(flow_label)
            heapq.heappush(self.heap, (estimated_size, flow_label))

            if len(self.heap) > k:
                heapq.heappop(self.heap)

        top_k_flows = [(flow_label, estimated_size) for estimated_size, flow_label in self.heap]
        return top_k_flows



def init_CMHeap(memory_kb):

    CM = CountMin.init_CountMin(memory_kb)

    return CMHeap(CM)
