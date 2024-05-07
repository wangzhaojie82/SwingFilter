

#ifndef ASKETCH_H
#define ASKETCH_H
#include "Sketch.h"

class ASketch : public Sketch{

public:
    char** flowlabels;
    // the number of new counters and old counters
    uint32_t w;
    uint32_t* new_counters;
    uint32_t* old_counters;

    uint32_t seed = 65123;

    Sketch* sketch;

    ASketch(float memory_kb, Sketch* sketch1);

    void update(const int packet_id, const char* flow_label, uint32_t weight= 1);

    int report(const char* flow_label);

    ~ASketch() {
        delete new_counters;
        delete old_counters;
        delete sketch;
    }
};


#endif
