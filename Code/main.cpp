#include "CountMedianSketch.hpp"     // For MultipleCMS
#include "RobustCountSketch.hpp"     // For BayesOptMultiCMS
#include "GraphMedianSketch.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <vector>
#include <string>
#include <cstdlib>

std::vector<std::vector<int>> load_csv(const std::string &filename) {
    std::vector<std::vector<int>> data;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        exit(1);
    }

    while (std::getline(file, line)) {
        std::vector<int> row;
        std::stringstream ss(line);
        std::string value;

        while (std::getline(ss, value, ',')) {
            row.push_back(std::stoi(value));
        }
        data.push_back(row);
    }

    return data;
}

std::vector<double> parse_double_list(const std::string &str) {
    std::vector<double> result;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, ',')) {
        result.push_back(std::stod(token));
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc != 8) {
        std::cerr << "Usage: " << argv[0] << " <dataset_name> <edge_file> <label_file> <rows> <buckets> <decays> <weights>\n";
        return 1;
    }

    std::string dataset_name = argv[1];
    std::string edge_file = argv[2];
    std::string label_file = argv[3];
    int rows = std::stoi(argv[4]);
    int buckets = std::stoi(argv[5]);
    std::vector<double> decay_factors = parse_double_list(argv[6]);
    std::vector<double> weights = parse_double_list(argv[7]);

    std::vector<std::vector<int>> edges = load_csv(edge_file);
    std::vector<std::vector<int>> label_data = load_csv(label_file);

    std::vector<int> src, dst, times, labels;
    for (const auto &row : edges) {
        src.push_back(row[0]);
        dst.push_back(row[1]);
        times.push_back(row[2]);
    }

    for (const auto &row : label_data) {
        labels.push_back(row[0]);
    }

    // === Run MultipleCMS ===
    {
        std::string algorithm = "MultipleCMS";
        std::cout << "\n--- Running MultipleCMS ---\n";
        auto start_time = std::chrono::high_resolution_clock::now();

        MultipleCMS multiple_cms(
            algorithm, dataset_name, rows, buckets, decay_factors, weights, src, dst, times, labels
        );
        multiple_cms.run();

        auto end_time = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        std::cout << "MultipleCMS execution time: " << elapsed << " ms\n";

        std::cout << "Scoring results on: " << dataset_name << std::endl;
        std::system("python3 -W ignore scores.py");
    }

    // === Run BayesOptMultiCMS ===
    {
        std::string algorithm = "BayesOptCMS";
        std::cout << "\n--- Running BayesOptMultiCMS (BO on Subset, Evaluation on Full) ---\n";
    
        // Subsample: Use only 15% of the data for BO
        double sample_fraction = 0.15;
        size_t sample_size = static_cast<size_t>(src.size() * sample_fraction);
    
        std::vector<int> src_sample(src.begin(), src.begin() + sample_size);
        std::vector<int> dst_sample(dst.begin(), dst.begin() + sample_size);
        std::vector<int> times_sample(times.begin(), times.begin() + sample_size);
        std::vector<int> labels_sample(labels.begin(), labels.begin() + sample_size);
    
        // Run BO just once on subset to get optimal decays/weights
        BayesOptMultiCMS bo_searcher(
            algorithm, dataset_name, rows, buckets,
            src_sample, dst_sample, times_sample, labels_sample
        );
    
        std::vector<double> optimal_decays = bo_searcher.get_optimal_decays();
        std::vector<double> optimal_weights = bo_searcher.get_optimal_weights();
    
        // Use full dataset with optimal params (skip BO)
        auto start_time = std::chrono::high_resolution_clock::now();
    
        BayesOptMultiCMS full_eval(
            algorithm, dataset_name, rows, buckets,
            src, dst, times, labels,
            optimal_decays, optimal_weights
        );
        full_eval.run();
    
        auto end_time = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        std::cout << "BayesOptMultiCMS full evaluation time: " << elapsed << " ms\n";
    
        std::cout << "Scoring results on: " << dataset_name << std::endl;
        std::system("python3 -W ignore scores.py");
    }

    // === Run GraphAnomalyDetector ===
    {
        std::string algorithm = "GraphAnomalyDetector";
        std::cout << "\n--- Running GraphAnomalyDetector ---\n";
        auto start_time = std::chrono::high_resolution_clock::now();

        GraphAnomalyDetector detector(
            algorithm, dataset_name, rows, buckets,
            decay_factors, weights, src, dst, times, labels
        );
        detector.run();

        auto end_time = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        std::cout << "GraphAnomalyDetector execution time: " << elapsed << " ms\n";

        std::cout << "Scoring results on: " << dataset_name << std::endl;
        std::system("python3 -W ignore scores.py graph_scores_labels.csv");
    }    

    return 0;
}
