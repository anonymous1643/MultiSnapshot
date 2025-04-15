#ifndef COUNT_MEDIAN_SKETCH_HPP
#define COUNT_MEDIAN_SKETCH_HPP

#include <vector>
#include <random>
#include <string>

class CountMedianSketch {
public:
    CountMedianSketch(int r, int b);
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

class MultipleCMS {
public:
    MultipleCMS(
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
    std::vector<CountMedianSketch> cms_snapshots;

    std::vector<double> get_scores();
};

#endif // COUNT_MEDIAN_SKETCH_HPP
