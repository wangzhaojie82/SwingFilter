#include <iostream>
#include <chrono>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include "./header/CountMin.h"
#include "./header/SwingFilter.h"
#include "./header/LogLogFilter.h"
using namespace std;


vector<pair<int, char*>> readDataSet(const string& file_path) {
    // load ip trace
    vector<pair<int, char*>> data;

    ifstream file(file_path);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << file_path << endl;
    }

    int total_packets = 0;
    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        string source_ip, dest_ip;
        iss >> source_ip >> dest_ip;

        char* source_ip_fixed = new char[KEY_LEN]; // 1 less for null terminator

        strncpy(source_ip_fixed, source_ip.c_str(), KEY_LEN - 1); // -1 to reserve space for null terminator

        memset(source_ip_fixed + std::min(static_cast<int>(source_ip.length()), KEY_LEN - 1), '0', KEY_LEN - std::min(static_cast<int>(source_ip.length()), KEY_LEN - 1));


        source_ip_fixed[KEY_LEN - 1] = '\0'; // null terminator

        // Fill data pair and add to vector
        data.push_back(make_pair(total_packets, source_ip_fixed));
        total_packets++;
    }

    return data;
}



void flow_size_estimation(Sketch* sketch, vector<pair<int, char*>> dataset){
    std::map<string, int> real_flows;
    for (const auto& data : dataset) {
        real_flows[data.second]++;
    }

    // Estimate flow sizes and calculate average relative error
    double total_relative_error = 0.0;
    uint32_t total_flows = 0;

    // Estimate flow sizes
    for (const auto& pair : real_flows) {
        int size = pair.second;
        int estimated_size = sketch->report(pair.first);
        double relative_error = std::abs(static_cast<float>(estimated_size) - static_cast<float>(size)) / static_cast<float>(size);
        total_relative_error += relative_error;
        total_flows++;
    }

    double average_relative_error = total_relative_error / total_flows;
    std::cout << "ARE: " << average_relative_error << std::endl;
}


void process(Sketch* sketch, const string& ip_trace){

    vector<pair<int, char*>> dataset = readDataSet(ip_trace);

    for (const auto& data : dataset) {
        sketch->update(data.first, data.second,1);
    }

    flow_size_estimation(sketch, dataset);
}




int main() {
    string ip_trace = "../data/your_dataset.txt";

    cout << "Hello! Program starts executing......" << endl;
    cout << "-------------------------------\n" << endl;

    float total_memory_kb = 200;
    cout << "Process: memory_kb="<< total_memory_kb << "KB | " << endl;
    float filter_memory_kb = total_memory_kb * 0.33;

    CountMin CM = CountMin(total_memory_kb - filter_memory_kb);
    SwingFilter SF = SwingFilter(filter_memory_kb, &CM);

    process(&SF,ip_trace);
    cout << "-------------------------------\n" << endl;

    return 0;
}
