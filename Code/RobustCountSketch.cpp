#include "RobustCountSketch.hpp"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <sstream>
#include <cmath>

RobustCountSketch::RobustCountSketch(int r, int b)
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

int RobustCountSketch::hash(int elem, int i) const {
    return (elem * hash_a[i] + hash_b[i]) % num_buckets;
}

void RobustCountSketch::insert(int elem, double weight) {
    for (int i = 0; i < num_rows; ++i) {
        count[i][hash(elem, i)] += weight;
    }
}

void RobustCountSketch::decay(double decay_factor) {
    for (auto &row : count) {
        for (auto &val : row) {
            val *= decay_factor;
        }
    }
}

double RobustCountSketch::get_count(int elem) const {
    std::vector<double> estimates(num_rows);

    for (int i = 0; i < num_rows; ++i) {
        estimates[i] = count[i][hash(elem, i)];
    }

    std::nth_element(estimates.begin(), estimates.begin() + num_rows / 2, estimates.end());
    return estimates[num_rows / 2]; // Fast median calculation
}

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <fstream>

BayesOptMultiCMS::BayesOptMultiCMS(
    std::string algorithm, std::string dataset_name, int rows, int buckets,
    std::vector<int> src, std::vector<int> dst, std::vector<int> times, std::vector<int> labels)
    : algorithm(algorithm), dataset_name(dataset_name), rows(rows), buckets(buckets),
      src(src), dst(dst), times(times), labels(labels) {

    // Run Bayesian Optimization to find the best decays and weights
    optimize_bayesian();
}

BayesOptMultiCMS::BayesOptMultiCMS(
    std::string algorithm, std::string dataset_name, int rows, int buckets,
    std::vector<int> src, std::vector<int> dst, std::vector<int> times, std::vector<int> labels,
    std::vector<double> decays, std::vector<double> weights)
    : algorithm(algorithm), dataset_name(dataset_name), rows(rows), buckets(buckets),
      src(src), dst(dst), times(times), labels(labels),
      optimal_decays(std::move(decays)), optimal_weights(std::move(weights)) {}

void BayesOptMultiCMS::optimize_bayesian() {
    std::vector<double> decay_candidates = {0.85, 0.88, 0.90, 0.92, 0.95, 0.99};
    std::vector<double> weight_candidates = {0.1, 0.2, 0.3, 0.4, 0.5};

    double best_auc = 0.0;
    std::vector<double> best_decays;
    std::vector<double> best_weights;

    std::random_device rd;
    std::mt19937 gen(42); // Fixed seed for stability

    for (int iter = 0; iter < 20; ++iter) {
        std::vector<double> selected_decays;
        std::vector<double> selected_weights;

        int num_decays = std::uniform_int_distribution<int>(1, 3)(gen);
        std::shuffle(decay_candidates.begin(), decay_candidates.end(), gen);
        for (int i = 0; i < num_decays; ++i) {
            selected_decays.push_back(decay_candidates[i]);
        }

        std::shuffle(weight_candidates.begin(), weight_candidates.end(), gen);
        for (int i = 0; i < num_decays; ++i) {
            selected_weights.push_back(weight_candidates[i]);
        }
        double sum_weights = std::accumulate(selected_weights.begin(), selected_weights.end(), 0.0);
        for (double &w : selected_weights) {
            w /= sum_weights;
        }

        double auc_score = evaluate_auc(selected_decays, selected_weights);
        if (std::isnan(auc_score)) {
            std::cerr << "AUC is NaN, skipping this iteration\n";
            continue;
        }

        if (auc_score > best_auc) {
            best_auc = auc_score;
            best_decays = selected_decays;
            best_weights = selected_weights;
        }
    }

    if (best_decays.empty() || best_weights.empty()) {
        std::cerr << "No optimal parameters found. Using default decay=0.95 and weight=1.0\n";
        best_decays = {0.95};
        best_weights = {1.0};
    }

    optimal_decays = best_decays;
    optimal_weights = best_weights;

    std::cout << "Optimal Decays: ";
    for (double d : optimal_decays) std::cout << d << " ";
    std::cout << "\nOptimal Weights: ";
    for (double w : optimal_weights) std::cout << w << " ";
    std::cout << std::endl;
}

double BayesOptMultiCMS::evaluate_auc(const std::vector<double>& decay_factors, const std::vector<double>& weights) {
    std::vector<RobustCountSketch> cms_snapshots;
    for (double decay : decay_factors) {
        cms_snapshots.emplace_back(rows, buckets);
    }

    std::vector<double> scores;
    int last_time = 0;

    for (size_t i = 0; i < src.size(); ++i) {
        if (times[i] - last_time > 0) {
            for (size_t j = 0; j < cms_snapshots.size(); ++j) {
                cms_snapshots[j].decay(decay_factors[j]);
            }
        }

        for (auto& cms : cms_snapshots) {
            cms.insert(src[i]);
            cms.insert(dst[i]);
        }

        std::vector<double> snapshot_scores;
        for (size_t j = 0; j < cms_snapshots.size(); ++j) {
            snapshot_scores.push_back(cms_snapshots[j].get_count(src[i]) * cms_snapshots[j].get_count(dst[i]));
        }

        double combined_score = 0.0;
        for (size_t j = 0; j < weights.size(); ++j) {
            combined_score += weights[j] * snapshot_scores[j];
        }

        scores.push_back(combined_score);
        last_time = times[i];
    }

    std::ofstream outfile("scores_labels.csv");
    outfile << "score,label\n";
    for (size_t i = 0; i < scores.size(); ++i) {
        outfile << scores[i] << "," << labels[i] << "\n";
    }
    outfile.close();

    // system("python3 compute_auc.py");

    std::ifstream infile("auc_result.txt");
    double auc;
    infile >> auc;
    infile.close();

    return auc;
}

std::vector<double> BayesOptMultiCMS::get_scores() {
    std::vector<RobustCountSketch> optimized_snapshots;
    for (double decay : optimal_decays) {
        optimized_snapshots.emplace_back(rows, buckets);
    }

    std::vector<double> scores;
    int last_time = 0;

    for (size_t i = 0; i < src.size(); ++i) {
        if (times[i] - last_time > 0) {
            for (size_t j = 0; j < optimized_snapshots.size(); ++j) {
                optimized_snapshots[j].decay(optimal_decays[j]);
            }
        }

        for (auto& cms : optimized_snapshots) {
            cms.insert(src[i]);
            cms.insert(dst[i]);
        }

        std::vector<double> snapshot_scores;
        for (size_t j = 0; j < optimized_snapshots.size(); ++j) {
            snapshot_scores.push_back(optimized_snapshots[j].get_count(src[i]) * optimized_snapshots[j].get_count(dst[i]));
        }

        double combined_score = 0.0;
        for (size_t j = 0; j < optimal_weights.size(); ++j) {
            combined_score += optimal_weights[j] * snapshot_scores[j];
        }

        scores.push_back(combined_score);
        last_time = times[i];
    }

    return scores;
}

void BayesOptMultiCMS::run() {
    std::vector<double> scores = get_scores();

    std::ofstream outfile("scores_labels.csv");
    outfile << "score,label\n";

    for (size_t i = 0; i < scores.size(); ++i) {
        outfile << scores[i] << "," << labels[i] << "\n";
    }
    outfile.close();

    std::cout << "Scores saved to scores_labels.csv. Use Python to compute AUC." << std::endl;
}
