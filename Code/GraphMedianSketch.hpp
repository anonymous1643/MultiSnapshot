#ifndef GRAPH_MEDIAN_SKETCH_HPP
#define GRAPH_MEDIAN_SKETCH_HPP

#include <vector>
#include <random>
#include <string>

class GraphMedianSketch {
public:
    GraphMedianSketch(int r, int b);
    void insert(int node, double weight = 1.0);
    void decay(double decay_factor);
    double get_count(int node) const;

private:
    int num_rows;
    int num_buckets;
    std::vector<int> hash_a;
    std::vector<int> hash_b;
    std::vector<std::vector<double>> count;

    int hash(int elem, int i) const;
};

class GraphAnomalyDetector {
public:
    GraphAnomalyDetector(
        std::string algorithm, std::string dataset_name,
        int rows, int buckets,
        std::vector<double> decay_factors, std::vector<double> weights,
        std::vector<int> src, std::vector<int> dst, std::vector<int> times, std::vector<int> labels
    );

    void run();

private:
    std::string algorithm;
    std::string dataset_name;
    int rows;
    int buckets;
    std::vector<double> decay_factors;
    std::vector<double> weights;
    std::vector<int> src;
    std::vector<int> dst;
    std::vector<int> times;
    std::vector<int> labels;
    std::vector<GraphMedianSketch> cms_snapshots;

    std::vector<double> get_scores();
};

#endif // GRAPH_MEDIAN_SKETCH_HPP
