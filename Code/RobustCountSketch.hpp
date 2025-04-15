#ifndef ROBUST_COUNT_SKETCH_HPP
#define ROBUST_COUNT_SKETCH_HPP

#include <vector>
#include <random>
#include <string>

class RobustCountSketch {
public:
    RobustCountSketch(int r, int b);
    void insert(int elem, double weight = 1.0);
    void decay(double decay_factor);
    double get_count(int elem) const;

private:
    int num_rows;
    int num_buckets;
    std::vector<int> hash_a;
    std::vector<int> hash_b;
    std::vector<std::vector<double>> count;

    int hash(int elem, int i) const;
};

#include <vector>
#include <string>

class BayesOptMultiCMS {
public:
    BayesOptMultiCMS(std::string algorithm, std::string dataset_name, int rows, int buckets,
                     std::vector<int> src, std::vector<int> dst, std::vector<int> times, std::vector<int> labels);

    BayesOptMultiCMS(std::string algorithm, std::string dataset_name, int rows, int buckets,
        std::vector<int> src, std::vector<int> dst,
        std::vector<int> times, std::vector<int> labels,
        std::vector<double> decays, std::vector<double> weights);
       
    void run();
    std::vector<double> get_optimal_decays() const { return optimal_decays; }
    std::vector<double> get_optimal_weights() const { return optimal_weights; }

private:
    std::string algorithm;
    std::string dataset_name;
    int rows;
    int buckets;
    std::vector<int> src;
    std::vector<int> dst;
    std::vector<int> times;
    std::vector<int> labels;

    std::vector<double> optimal_decays;
    std::vector<double> optimal_weights;
    std::vector<RobustCountSketch> cms_snapshots;
    
    std::vector<double> get_scores();
    
    void optimize_bayesian();

    double evaluate_auc(const std::vector<double>& decay_factors, const std::vector<double>& weights);
};

#endif // ROBUST_COUNT_SKETCH_HPP
