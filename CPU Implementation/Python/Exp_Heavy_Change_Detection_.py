import concurrent.futures
import time
import re
import Packet

from filters import BounceFilter
from sketches import MVSketch, ElasticSketch


def calculate_metrics(true_heavy_change, detected_heavy_change, all_flows_arr, heavy_flows_arr):
    '''
    :param true_heavy_change: a flow set of true heavy change
    :param detected_heavy_change: a flow set of detected heavy change
    :param all_flows_arr: a list of two dictionaries, each dict contains the true size of flows for an epoch
    :param heavy_flows_arr: a list of two dictionaries, each dict contains the estimated size of detected heavy flows for an epoch
    '''

    true_positives = 0

    # Only calculate ARE for true positives
    total_relative_error = 0

    for flow_label in detected_heavy_change:
        if flow_label in true_heavy_change:
            true_positives += 1

            estimated_size = 0
            if flow_label in heavy_flows_arr[0].keys():
                estimated_size = heavy_flows_arr[0][flow_label]
                epoch_appear = 0

            elif flow_label in heavy_flows_arr[1].keys():
                estimated_size = heavy_flows_arr[1][flow_label]
                epoch_appear = 1
            else:
                epoch_appear = -1

            if epoch_appear > -1:
                true_size = 0
                if flow_label in all_flows_arr[epoch_appear].keys():
                    true_size = all_flows_arr[epoch_appear][flow_label]

                rela_err = abs(estimated_size - true_size) / true_size
                total_relative_error += rela_err


    average_relative_error, precision, recall, f1 = 0, 0, 0, 0

    # Calculate average relative error
    if true_positives > 0:
        average_relative_error = round(total_relative_error / true_positives, 3)

    # Calculate Precision
    if len(detected_heavy_change) > 0:
        precision = round(true_positives / len(detected_heavy_change), 3)

    # Calculate recall
    if len(true_heavy_change) > 0:
        recall = round(true_positives / len(true_heavy_change), 3)

    # Calculate F1 score
    if precision + recall > 0:
        f1 = round(2 * (precision * recall) / (precision + recall), 3)

    return average_relative_error, precision, recall, f1


def heavy_change_d(**kwargs):

    traffic_traces = kwargs['traffic_traces']

    memory_kb = kwargs['memory_kb']

    # regular expression to match IPv4 address
    pattern = r"\b(?:\d{1,3}\.){3}\d{1,3}\b"

    all_flows_arr = []  # list of two dictionaries, each dict records real size of flows in one epoch
    sketch_epoch_arr = [] # store the sketch of each epoch
    filter_epoch_arr = [] # store the filter of each epoch

    for i in range(2):
        # Processing of each epoch
        filter_ = BounceFilter.init_BounceFilter(memory_kb * 0.2)
        sketch_ = ElasticSketch.init_ElasticSketch(memory_kb * 0.8)
        # sketch_ = MVSketch.init_MVSketch(memory_kb * 0.8)

        all_flows = {}
        total_packets = 0  # total number of all packets

        with open(traffic_traces[i], 'r', encoding='utf-8') as f:
            for line in f:
                tmp = line.split()
                # filter the invalid IPv4 packets
                if not re.match(pattern, tmp[0]) or not re.match(pattern, tmp[1]):
                    continue

                packet = Packet.Packet(total_packets, tmp[0], tmp[1])

                # to count all packets processed
                total_packets += 1
                if packet.flow_label in all_flows.keys():
                    all_flows[packet.flow_label] += 1
                else:
                    all_flows[packet.flow_label] = 1

                succ = filter_.update(packet)
                if succ:
                    continue
                else:
                    sketch_.update(packet)

        all_flows_arr.append(all_flows)
        sketch_epoch_arr.append(filter_)
        filter_epoch_arr.append(sketch_)


    # find heavy change
    heavy_change_threshold = kwargs['heavy_change_threshold']

    filename = f'HHC_Memory={memory_kb}_heavy_change_detection.txt'
    # Write results to a text file
    with open(filename, 'w') as file:

        for threshold in heavy_change_threshold:

            true_heavy_change = set()
            detected_heavy_change = set()

            checked_flows = set()
            # find true heavy change
            for flow in all_flows_arr[0].keys():
                size_epoch_1 = all_flows_arr[0][flow]

                size_epoch_2 = 0
                if flow in all_flows_arr[1].keys():
                    size_epoch_2 = all_flows_arr[1][flow]

                checked_flows.add(flow)
                if abs(size_epoch_1 - size_epoch_2) > threshold:
                    true_heavy_change.add(flow)

            for flow in all_flows_arr[1].keys():
                if flow in checked_flows:
                    continue
                size_epoch_2 = all_flows_arr[1][flow]

                size_epoch_1 = 0
                if flow in all_flows_arr[0].keys():
                    size_epoch_1 = all_flows_arr[0][flow]

                if abs(size_epoch_2 - size_epoch_1) > threshold:
                    true_heavy_change.add(flow)

            # find detected heavy change
            heavy_flows_arr = []
            for i in range(2):
                heavy_flow_epoch = dict(sketch_epoch_arr[i].all_heavy_flows())
                for flow, size in heavy_flow_epoch.items():
                    size_in_filter, _ = filter_epoch_arr[i].report(flow)
                    heavy_flow_epoch[flow] = size + size_in_filter

                heavy_flows_arr.append(heavy_flow_epoch)


            filter_epoch_1, filter_epoch_2 = filter_epoch_arr[0], filter_epoch_arr[1]
            heavy_flow_epoch_1, heavy_flow_epoch_2 = heavy_flows_arr[0], heavy_flows_arr[1]

            checked_heavy_flows = set()
            for flow, size in heavy_flow_epoch_1.items():
                estimated_size_epoch_1 = size

                if flow in heavy_flow_epoch_2.keys():
                    estimated_size_epoch_2 = heavy_flow_epoch_2[flow]
                else:
                    size_in_filter, large_flag = filter_epoch_2.report(flow)
                    estimated_size_epoch_2 = size_in_filter
                    if large_flag:
                        estimated_size_epoch_2 += sketch_epoch_arr[1].report(flow)

                checked_heavy_flows.add(flow)
                if abs(estimated_size_epoch_1 - estimated_size_epoch_2) > threshold:
                    detected_heavy_change.add(flow)

            for flow, size in heavy_flow_epoch_2.items():
                if flow in checked_heavy_flows:
                    continue
                estimated_size_epoch_2 = size

                if flow in heavy_flow_epoch_1.keys():
                    estimated_size_epoch_1 = heavy_flow_epoch_1[flow]
                else:
                    size_in_filter, large_flag = filter_epoch_1.report(flow)
                    estimated_size_epoch_1 = size_in_filter
                    if large_flag:
                        estimated_size_epoch_1 += sketch_epoch_arr[0].report(flow)

                if abs(estimated_size_epoch_2 - estimated_size_epoch_1) > threshold:
                    detected_heavy_change.add(flow)

            ARE, PR, CR, F1 = calculate_metrics(true_heavy_change, detected_heavy_change, all_flows_arr,
                                                heavy_flows_arr)

            file.write(f"Heavy Change threshold = {threshold}  |  ARE = {ARE}  |  Precision = {PR}  |  "
                       f"Recall = {CR}  |  F1 = {F1} \n")
            file.write(f"\n")


def run_method(method, **kwargs):
    method(**kwargs)


def run_parallel():

    traffic_traces = ['./data/your_dataset.txt']

    heavy_change_threshold = [100, 500, 1000, 2000, 4000, 6000, 8000, 10000]

    with concurrent.futures.ProcessPoolExecutor() as executor:
        futures = []
        for memory_kb in [100, 200]:

            futures.append(
                executor.submit(run_method, heavy_change_d,
                                traffic_traces=traffic_traces,
                                heavy_change_threshold=heavy_change_threshold,
                                memory_kb=memory_kb))

        concurrent.futures.wait(futures)


if __name__ == '__main__':

    print("Program execution started.")
    start_time = time.time()

    run_parallel()

    end_time = time.time()
    execution_time = end_time - start_time
    formatted_time = time.strftime('%H:%M:%S', time.gmtime(execution_time))

    print(f"\nTotal execution time is: {formatted_time}")