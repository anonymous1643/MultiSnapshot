#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: bash demo.sh <dataset_name>"
    echo "Available datasets: darpa, iscx, ids2018, ddos2019"
    exit 1
fi

DATASET=$1
LOWER=$(echo "$DATASET" | tr '[:upper:]' '[:lower:]')
DATA_DIR="../data/$LOWER"

ROWS=4
BUCKETS=128
DECAYS=""
WEIGHTS=""

case "$LOWER" in
    darpa)
        DECAYS="0.70,0.80,0.90"
        WEIGHTS="0.5,0.3,0.2"
        EDGE_FILE="../data/darpa/darpa-Data.csv"
        LABEL_FILE="../data/darpa/darpa-Label.csv"
        ;;
    iscx)
        DECAYS="0.90,0.95,0.99"
        WEIGHTS="0.5,0.3,0.2"
        EDGE_FILE="../data/iscx/iscx-Data.csv"
        LABEL_FILE="../data/iscx/iscx-Label.csv"
        ;;
    ids2018|ddos2019)
        echo "Extracting $DATASET from ZIP via Python..."
        python3 extract_from_zip.py "$LOWER" || { echo "Failed to extract $DATASET"; exit 1; }

        DECAYS="0.90,0.95,0.99"
        WEIGHTS="0.5,0.3,0.2"
        EDGE_FILE="./tmp_data/${LOWER}_Data.csv"
        LABEL_FILE="./tmp_data/${LOWER}_Label.csv"
        ;;
    *)
        echo "Unknown dataset: $DATASET"
        exit 1
        ;;
esac

# Compile the code
echo "Compiling..."
g++ -O3 -march=native -flto -static-libstdc++ -static-libgcc \
    CountMedianSketch.cpp RobustCountSketch.cpp GraphMedianSketch.cpp main.cpp \
    -o run_cms

# Run the program
echo "Running MultipleCMS, BayesOptMultiCMS, and GraphAnomalyDetector on: $DATASET"
./run_cms "$LOWER" "$EDGE_FILE" "$LABEL_FILE" "$ROWS" "$BUCKETS" "$DECAYS" "$WEIGHTS"

# Cleanup
rm -rf tmp_data
