#include "CountMedianSketch.hpp"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <sstream>
#include <cmath>
#include <chrono>  // For timing

CountMedianSketch::CountMedianSketch(int r, int b)
    : num_rows(r), num_buckets(b), count(r, std::vector<double>(b, 0.0)) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist_a(1, b - 1);
    std::uniform_int_distribution<int> dist_b(0, b - 1);

    for (int i = 0; i < r; ++i) {
        hash_a.push_back(dist_a(gen));
        hash_b.push_back(dist_b(gen));
    }
}

int CountMedianSketch::hash(int elem, int i) const {
    return (elem * hash_a[i] + hash_b[i]) % num_buckets;
}

void CountMedianSketch::insert(int elem, double weight) {
    for (int i = 0; i < num_rows; ++i) {
        count[i][hash(elem, i)] += weight;
    }
}

void CountMedianSketch::decay(double decay_factor) {
    for (auto &row : count) {
        for (auto &val : row) {
            val *= decay_factor;
        }
    }
}

double CountMedianSketch::get_count(int elem) const {
    std::vector<double> estimates(num_rows);
    
    for (int i = 0; i < num_rows; ++i) {
        estimates[i] = count[i][hash(elem, i)];
    }
    
    std::nth_element(estimates.begin(), estimates.begin() + num_rows / 2, estimates.end());
    return estimates[num_rows / 2]; // Fast median calculation
}


MultipleCMS::MultipleCMS(
    std::string algorithm, std::string dataset_name, 
    int rows, int buckets, 
    std::vector<double> decay_factors, std::vector<double> weights, 
    std::vector<int> src, std::vector<int> dst, std::vector<int> times, std::vector<int> labels
) : algorithm(algorithm), dataset_name(dataset_name), rows(rows), buckets(buckets), 
    decay_factors(decay_factors), weights(weights), 
    src(src), dst(dst), times(times), labels(labels) {
    
    for (size_t i = 0; i < decay_factors.size(); ++i) {
        cms_snapshots.emplace_back(rows, buckets);
    }
}

std::vector<double> MultipleCMS::get_scores() {
    std::vector<double> scores;
    int last_time = 0;

    // Open file for runtime logging
    std::ofstream time_log("runtime_vs_edges.csv");
    
    time_log << "num_edges,time_ms\n";  // CSV header

    auto edge_start_time = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < src.size(); ++i) {
        // Apply decay if needed
        if (times[i] - last_time > 0) {
            for (size_t j = 0; j < decay_factors.size(); ++j) {
                cms_snapshots[j].decay(decay_factors[j]);
            }
        }

        // Insert edges into sketches
        for (auto &cms : cms_snapshots) {
            cms.insert(src[i]);
            cms.insert(dst[i]);
        }

        // Compute scores
        std::vector<double> snapshot_scores;
        for (auto &cms : cms_snapshots) {
            snapshot_scores.push_back(cms.get_count(src[i]) * cms.get_count(dst[i]));
        }

        double combined_score = 0.0;
        for (size_t j = 0; j < weights.size(); ++j) {
            combined_score += weights[j] * snapshot_scores[j];
        }
        scores.push_back(combined_score);

        last_time = times[i];

        // Log runtime every 1,000,000 edges
        if (i % 1000000 == 0 || i == src.size() - 1) {
            auto edge_current_time = std::chrono::high_resolution_clock::now();
            double elapsed_ms = std::chrono::duration<double, std::milli>(edge_current_time - edge_start_time).count();
            time_log << i + 1 << "," << elapsed_ms << "\n";
            edge_start_time = edge_current_time;  // Reset timer
        }
    }

    time_log.close();
    return scores;
}

void MultipleCMS::run() {
    std::vector<double> scores = get_scores();

    // Write scores and labels to a file for Python AUC calculation
    std::ofstream outfile("scores_labels.csv");
    outfile << "score,label\n"; 

    for (size_t i = 0; i < scores.size(); ++i) {
        outfile << scores[i] << "," << labels[i] << "\n";
    }
    outfile.close();

    std::cout << "Scores saved to scores_labels.csv. Use Python to compute AUC." << std::endl;
}
