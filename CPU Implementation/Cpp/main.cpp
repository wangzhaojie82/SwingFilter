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

    unordered_map<string, int> real_flows;
    unordered_map<string, int> estimated_flows;
    set<string> processed;

    for (const auto& data : dataset) {

        if (real_flows.count(data.second) == 0) {
            real_flows[data.second] = 1;
        } else {
            real_flows[data.second]++;
        }

        if (processed.find(data.second) != processed.end()) {
            continue;
        }else{
            int estimated_size = sketch->report(data.second);
            estimated_flows[data.second] = estimated_size;
            processed.insert(data.second);
        }
    }

    // Estimate flow sizes and calculate average relative error
    double total_relative_error = 0.0;
    int total_flows = 0;

    // Estimate flow sizes
    for (const auto& pair : real_flows) {
        string flow_label_str = pair.first;
        int real_size = pair.second;
        int estimated_size = estimated_flows[flow_label_str];

        double relative_error = fabs((estimated_size - real_size) / static_cast<double>(real_size));
        total_relative_error += relative_error;
        total_flows++;
    }

    double average_relative_error = total_relative_error / total_flows;
    std::cout << "ARE: " << average_relative_error << std::endl;

}



void process(Sketch* sketch, const string& ip_trace){
    /*
     * process the dataset on path: ip_trace
     */

    //vector<pair<char*, char*>> dataset = readDataSet(ip_trace);
    vector<pair<int, char*>> dataset = readDataSet(ip_trace);

    auto start_time = chrono::steady_clock::now();
    for (const auto& data : dataset) {
        sketch->update(data.first, data.second,1);
    }
    auto end_time = chrono::steady_clock::now();

    auto elapsed_time = chrono::duration_cast<chrono::seconds>(end_time - start_time).count();
    double throughput_mpps = static_cast<double>(dataset.size()) / elapsed_time / 1000000.0;
    cout << "Update Throughput: " << throughput_mpps << " Mpps" << endl;


    // ----------

    auto start_time_query = chrono::steady_clock::now();
    for (const auto& data : dataset) {
        sketch->report(data.second);
    }
    auto end_time_query = chrono::steady_clock::now();

    auto elapsed_time_query = chrono::duration_cast<chrono::seconds>(end_time_query - start_time_query).count();
    double throughput_mpps_query = static_cast<double>(dataset.size()) / elapsed_time_query / 1000000.0;
    cout << "Query Throughput: " << throughput_mpps_query << " Mfps" << endl;

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
