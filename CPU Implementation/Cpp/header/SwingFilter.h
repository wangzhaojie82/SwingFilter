#ifndef BOUNCEFILTER_H
#define BOUNCEFILTER_H


#include "MurmurHash3.h"
#include "Sketch.h"

class SwingFilter : public Sketch{
private:
    int d; // num of hash per layer
    int bits[2]; // counter bits per layer
    int max_positive[2];
    int min_negative[2];
    uint32_t num_counters[2]; // num of counters per layer
    int** counters; // filter, array of int arrays
    Sketch* skt1;

public:
    SwingFilter(float memory_kb, Sketch* sketch1);

    int hash_s(const char* flow_label, uint32_t& counter_index);

    void update(const int packet_id, const char* flow_label, uint32_t weight= 1);

    int report(const char* flow_label);

    // Destructor to free memory
    ~SwingFilter() {
        delete[] counters[0];
        delete[] counters[1];
    }

};

#endif
