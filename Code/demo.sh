#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: bash demo.sh <dataset_name>"
    echo "Available datasets: darpa, iscx, ids2018, ddos2019"
    exit 1
fi

DATASET=$1
LOWER=$(echo "$DATASET" | tr '[:upper:]' '[:lower:]')
DATA_DIR="../data/$LOWER"

EDGE_FILE="$DATA_DIR/${LOWER}-Data.csv"
LABEL_FILE="$DATA_DIR/${LOWER}-Label.csv"
ROWS=4
BUCKETS=128
DECAYS=""
WEIGHTS=""

case "$LOWER" in
    darpa)
        DECAYS="0.10,0.20,0.90"
        WEIGHTS="0.0,0.0,1.0"
        ;;
    iscx)
        DECAYS="0.05,0.15,0.80"
        WEIGHTS="0.1,0.2,0.7"
        ;;
    ids2018)
        DECAYS="0.2,0.4,0.6"
        WEIGHTS="0.2,0.3,0.5"
        ;;
    ddos2019)
        DECAYS="0.25,0.5,0.75"
        WEIGHTS="0.3,0.3,0.4"
        ;;
    *)
        echo "Unknown dataset: $DATASET"
        exit 1
        ;;
esac

# Build inside the code folder
echo "Compiling..."
g++ -O3 -march=native -flto -static-libstdc++ -static-libgcc CountMedianSketch.cpp main.cpp -o CountMedianSketch

# Run the program
echo "Running CMS on: $DATASET"
./CountMedianSketch "$LOWER" "$EDGE_FILE" "$LABEL_FILE" "$ROWS" "$BUCKETS" "$DECAYS" "$WEIGHTS" 2>/dev/null || true

echo "Scoring CMS on: $DATASET"
python -W ignore scores.py