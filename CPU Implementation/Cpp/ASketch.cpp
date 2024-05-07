
#include "header/ASketch.h"

ASketch::ASketch(float memory_kb, Sketch* sketch1){
    sketch =sketch1;
    w = memory_kb * 1024 * 8 / 96;

    flowlabels = new char *[w];
    this->new_counters = new uint32_t[this->w]{0};
    this->old_counters = new uint32_t[this->w]{0};
    for (int i = 0; i < w; ++i)
        flowlabels[i] = "";
}

void ASketch::update(const int packet_id, const char* flow_label, uint32_t weight) {
    uint32_t hashIndex, hashValue;
    uint32_t backValue, min_index, min_value = 0x7fffffff;
    MurmurHash3_x86_32(flow_label, KEY_LEN, seed, &hashValue);
    hashIndex = hashValue % this->w;
    if (memcmp(flowlabels[hashIndex], "", KEY_LEN) == 0) {
        flowlabels[hashIndex] = new char[KEY_LEN];
        memcpy(flowlabels[hashIndex], flow_label,KEY_LEN);
        new_counters[hashIndex] += weight;
        old_counters[hashIndex]  = 0;
    } else if (memcmp(flowlabels[hashIndex], flow_label, KEY_LEN) == 0)
        new_counters[hashIndex] += weight;
    else {
        sketch->update(packet_id, flow_label, 1);
        backValue = sketch->report(flow_label);
        for (int i = 0; i < w; ++i)
            if (new_counters[i] < min_value) {
                min_value = new_counters[i];
                min_index = i;
            }
        if (backValue > min_value) {
            if (new_counters[min_index] - old_counters[min_index] > 0)
                sketch->update(0, flowlabels[min_index], new_counters[min_index] - old_counters[min_index]);
            if (flowlabels[min_index] == "")
                flowlabels[min_index] = new char[KEY_LEN];
            memcpy(flowlabels[min_index], flow_label, KEY_LEN);
            new_counters[min_index] = backValue;
            old_counters[min_index] = backValue;
        }
    }
}

int ASketch::report(const char *flow_label){
    uint32_t hashIndex, hashValue;
    MurmurHash3_x86_32(flow_label, KEY_LEN, seed, &hashValue);
    hashIndex = hashValue % this->w;
    if (memcmp(flowlabels[hashIndex], flow_label, KEY_LEN) == 0)
        return new_counters[hashIndex];
    else
        return sketch->report(flow_label);
}