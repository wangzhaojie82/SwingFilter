
#ifndef COUNTMIN_H
#define COUNTMIN_H

#include "Sketch.h"

class CountMin : public Sketch{
    private:
        int depth;
        int width;
        uint32_t counter_bits;
        std::vector<std::vector<uint32_t>> counters;

    public:
        CountMin(float memory_kb);

        void update(const int packet_id, const char* flow_label, uint32_t weight= 1);

        int report(const char* flow_label);

};


#endif
