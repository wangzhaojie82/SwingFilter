import concurrent.futures
import time
import re
import Packet

from filters import BounceFilter, ASketch
from sketches import SpaceSaving, CMHeap, CountMin


def calculate_ARE_PR(top_k, top_k_flows, all_flows):


    # Sort all_flows by flow size
    sorted_all_flows = sorted(all_flows.items(), key=lambda x: x[1], reverse=True)

    # Determine true positives (flows in top_k_flows that are among top k in all_flows)
    true_positives = 0

    # Calculate ARE for true positives
    total_relative_error = 0
    for flow_label, estimated_size in top_k_flows:

        if flow_label in dict(sorted_all_flows[:top_k]):
            true_size = all_flows.get(flow_label, 0)
            if true_size > 0:
                true_positives += 1  # 统计真正例个数
                relative_error = abs(estimated_size - true_size) / true_size
                total_relative_error += relative_error

    average_relative_error, precision = 0, 0
    if true_positives > 0:
        average_relative_error = round(total_relative_error / true_positives, 3)

    if top_k > 0:
        precision = round(true_positives / top_k, 3)

    return average_relative_error, precision


def space_saving_test(**kwargs):

    traffic_traces = kwargs['traffic_traces']
    top_k_list = kwargs['top_k_list']

    memory_kb = kwargs['memory_kb']
    memory_kb_filter = memory_kb * 0.2
    memory_kb_ss = memory_kb- memory_kb_filter

    filter_ = BounceFilter.init_BounceFilter(memory_kb_filter)

    SS = SpaceSaving.init_SpaceSaving(memory_kb_ss)

    all_flows = {}
    total_packets = 0  # total number of all packets
    # regular expression to match IPv4 address
    pattern = r"\b(?:\d{1,3}\.){3}\d{1,3}\b"
    for traffic_trace in traffic_traces:
        with open(traffic_trace, 'r', encoding='utf-8') as f:
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
                    SS.update(packet)


    filename = f'SS+F_Memory={memory_kb}_top_k_task_.txt'
    # Write results to a text file
    with open(filename, 'w') as file:
        for top_k in top_k_list:

            top_k_flows = SS.query_with_filter(filter_, top_k)
            ARE, PR = calculate_ARE_PR(top_k,top_k_flows,all_flows)

            file.write(f"K = {top_k}  |  ARE = {ARE}  |  Precision = {PR} \n")
            file.write(f"\n")


def cm_heap(**kwargs):

    traffic_traces = kwargs['traffic_traces']
    memory_kb = kwargs['memory_kb']

    top_k_list = kwargs['top_k_list']

    filename = f'CMHeap_Memory={memory_kb}_top_k_task.txt'
    # Write results to a text file
    with open(filename, 'w') as file:
        for top_k in top_k_list:

            heap_memory = round((64 * top_k) / (1024 * 8))
            cm_memory = memory_kb - heap_memory

            CMH = CMHeap.init_CMHeap(cm_memory)

            all_flows = {}
            total_packets = 0  # total number of all packets
            # regular expression to match IPv4 address
            pattern = r"\b(?:\d{1,3}\.){3}\d{1,3}\b"
            for traffic_trace in traffic_traces:
                with open(traffic_trace, 'r', encoding='utf-8') as f:
                    for line in f:
                        tmp = line.split()
                        # filter the invalid packets
                        if not re.match(pattern, tmp[0]) or not re.match(pattern, tmp[1]):
                            continue

                        packet = Packet.Packet(total_packets, tmp[0], tmp[1])

                        # to count all packets processed
                        total_packets += 1
                        if packet.flow_label in all_flows.keys():
                            all_flows[packet.flow_label] += 1
                        else:
                            all_flows[packet.flow_label] = 1

                        CMH.update(packet)


            top_k_flows = CMH.query(top_k)
            ARE, PR = calculate_ARE_PR(top_k, top_k_flows, all_flows)

            file.write(f"K = {top_k}  |  ARE = {ARE}  |  Precision = {PR} \n")
            file.write(f"\n")


def CM_AS_top_k(**kwargs):
    '''
    CountMin + ASketch to query top-k
    '''


    traffic_traces = kwargs['traffic_traces']

    memory_kb = kwargs['memory_kb']

    top_k_list = kwargs['top_k_list']

    # init filter component of ASketch
    AS = ASketch.init_ASketch(memory_kb * 0.5)
    CM = CountMin.init_CountMin(memory_kb * 0.5)

    all_flows = {}
    total_packets = 0

    # regular expression to match IPv4 address
    pattern = r"\b(?:\d{1,3}\.){3}\d{1,3}\b"
    for traffic_trace in traffic_traces:
        with open(traffic_trace, 'r', encoding='utf-8') as f:
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

                # first update in filter component
                succ = AS.update(packet)
                if succ:
                    continue
                else:
                    CM.update(packet)

                    # Check if swapping is needed

                    # Find the item in filter with the minimum new_count
                    # min_item_label: the flow label of item with the minimum new_count
                    # min_item_info: a tuple (new_count, old_count) of this item
                    min_item_label, min_item_info = min(AS.filter.items(), key=lambda x: x[1][0])

                    estimated_size = CM.report(packet.flow_label)

                    if estimated_size > min_item_info[0]:

                        # Update the sketch with packets from the minimum item
                        num_to_update = round(min_item_info[0] - min_item_info[1])
                        CM.update_with_weight(min_item_label, num_to_update)

                        # Delete the minimum item
                        del AS.filter[min_item_label]

                        # Add the new item with the updated flow_label and size
                        AS.filter[packet.flow_label] = (estimated_size, estimated_size)



    filename = f'AS+CM_Memory={memory_kb}_top_k_task.txt'
    # Write results to a text file
    with open(filename, 'w') as file:
        for top_k in top_k_list:

            top_k_flows = AS.top_k_query(top_k)

            ARE, PR = calculate_ARE_PR(top_k, top_k_flows, all_flows)

            file.write(f"K = {top_k}  |  ARE = {ARE}  |  Precision = {PR} \n")
            file.write(f"\n")


def run_method(method, **kwargs):
    method(**kwargs)


def run_parallel():

    traffic_traces = ['./data/your_dataset.txt']

    top_k_list = [100, 500, 1000, 2000, 4000, 6000, 8000, 10000]

    with concurrent.futures.ProcessPoolExecutor() as executor:

        futures = []


        for memory_kb in [100, 200]:

            futures.append(
                executor.submit(run_method, cm_heap, traffic_traces=traffic_traces,
                                top_k_list=top_k_list, memory_kb=memory_kb))

        concurrent.futures.wait(futures)


if __name__ == '__main__':

    print("Program execution started.")
    start_time = time.time()

    run_parallel()

    end_time = time.time()
    execution_time = end_time - start_time
    formatted_time = time.strftime('%H:%M:%S', time.gmtime(execution_time))

    print(f"\nTotal execution time is: {formatted_time}")