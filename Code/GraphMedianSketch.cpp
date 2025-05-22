#include "GraphMedianSketch.hpp"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <sstream>
#include <cmath>
#include <chrono>

GraphMedianSketch::GraphMedianSketch(int r, int b)
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

int GraphMedianSketch::hash(int elem, int i) const {
    return (elem * hash_a[i] + hash_b[i]) % num_buckets;
}

void GraphMedianSketch::insert(int node, double weight) {
    for (int i = 0; i < num_rows; ++i) {
        count[i][hash(node, i)] += weight;
    }
}

void GraphMedianSketch::decay(double decay_factor) {
    for (auto &row : count) {
        for (auto &val : row) {
            val *= decay_factor;
        }
    }
}

double GraphMedianSketch::get_count(int node) const {
    std::vector<double> estimates(num_rows);
    for (int i = 0; i < num_rows; ++i) {
        estimates[i] = count[i][hash(node, i)];
    }
    std::nth_element(estimates.begin(), estimates.begin() + num_rows / 2, estimates.end());
    return estimates[num_rows / 2];
}

GraphAnomalyDetector::GraphAnomalyDetector(
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

std::vector<double> GraphAnomalyDetector::get_scores() {
    std::vector<double> scores;
    int last_time = 0;

    std::ofstream time_log("runtime_vs_nodes.csv");
    time_log << "num_events,time_ms\n";

    auto process_start_time = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < src.size(); ++i) {
        if (times[i] - last_time > 0) {
            for (size_t j = 0; j < decay_factors.size(); ++j) {
                cms_snapshots[j].decay(decay_factors[j]);
            }
        }

        for (auto &cms : cms_snapshots) {
            cms.insert(src[i]);
            cms.insert(dst[i]);
        }

        std::vector<double> snapshot_scores;
        for (auto &cms : cms_snapshots) {
            double score_src = cms.get_count(src[i]);
            double score_dst = cms.get_count(dst[i]);
            snapshot_scores.push_back(score_src * score_dst);
        }

        double combined_score = 0.0;
        for (size_t j = 0; j < weights.size(); ++j) {
            combined_score += weights[j] * snapshot_scores[j];
        }

        scores.push_back(combined_score);
        last_time = times[i];

        if (i % 1000000 == 0 || i == src.size() - 1) {
            auto current_time = std::chrono::high_resolution_clock::now();
            double elapsed_ms = std::chrono::duration<double, std::milli>(current_time - process_start_time).count();
            time_log << i + 1 << "," << elapsed_ms << "\n";
            process_start_time = current_time;
        }
    }

    time_log.close();
    return scores;
}

void GraphAnomalyDetector::run() {
    std::vector<double> scores = get_scores();

    std::ofstream outfile("scores_labels.csv");
    outfile << "score,label\n";
    for (size_t i = 0; i < scores.size(); ++i) {
        outfile << scores[i] << "," << labels[i] << "\n";
    }

    outfile.close();
    std::cout << "Graph anomaly scores saved to scores_labels.csv. Use Python to compute AUC." << std::endl;
}
