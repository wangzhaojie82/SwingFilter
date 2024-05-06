
#include "header/CountMin.h"

CountMin::CountMin(float memory_kb) {

    this->counter_bits = 32;
    this->depth = 4;

    uint32_t total_counters = static_cast<uint32_t>(std::round(memory_kb * 1024 * 8 / this->counter_bits));

    //this->depth = static_cast<int>(std::log2(total_counters));

    width = static_cast<int>(std::round(total_counters / depth));

    counters = std::vector<std::vector<uint32_t>>(depth, std::vector<uint32_t>(width, 0));
}



void CountMin::update(const int packet_id, const char* flow_label, uint32_t weight) {
    uint32_t hash_value = 0;
    for (int i = 0; i < depth; i++) {
        MurmurHash3_x86_32(flow_label, KEY_LEN, i, &hash_value);
        uint32_t j = hash_value % width;
        uint32_t current_value = counters[i][j];
        uint32_t threshold = (1 << 32) - 1;
        uint32_t next_value = current_value + weight;
        if (next_value > threshold) {
            continue;
        }
        counters[i][j] = next_value;
    }
}


int CountMin::report(const char* flow_label) {

    int min_value = 0x7FFFFFFF;
    uint32_t hash_value = 0;
    for (int i = 0; i < depth; i++) {
        MurmurHash3_x86_32(flow_label, KEY_LEN, i, &hash_value);
        uint32_t j = hash_value % width;
        int val = counters[i][j];
        if (val < min_value) {
            min_value = val;
        }
    }
    return min_value;
}
