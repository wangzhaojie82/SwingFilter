

#ifndef BOUNCEFILTER_CPP_V2_LOGLOGFILTER_H
#define BOUNCEFILTER_CPP_V2_LOGLOGFILTER_H


#include "MurmurHash3.h"
#include "Sketch.h"

class LogLogFilter : public Sketch{
private:
    int m;
    int r;
    int f;
    int delta;
    double phi;
    Sketch* sketch;
    std::vector<int8_t> R;
    std::vector<int> seeds;

public:
    LogLogFilter(float memory_kb, Sketch* sketch1);

    int get_leftmost(uint32_t random_val);

    void update(const int packet_id, const char* flow_label, uint32_t weight= 1);

    int report(const char* flow_label);

};


#endif
