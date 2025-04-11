# MultiSnapshot

Implementation of:

**A Multi-Snapshot Sketching Framework for Anomaly Detection in Streaming Graphs**  

We propose a novel sketch-based framework for real-time anomaly detection in streaming graphs. Unlike existing approaches that rely on a single decay schedule, our method maintains multiple Count Sketch instances with varying decay rates to capture anomalies across different temporal scales. Our approach detects both edge-level and subgraph-level anomalies while maintaining constant memory and time complexity.

---

## Key Features

- **Multi-timescale modeling** using parallel Count Sketch structures with independently decaying weights
- **Edge-level anomaly detection** with Bayesian-optimized decay and weight settings
- **Subgraph anomaly detection** using higher-order sketch hashing and decay
- **Constant time and memory complexity**, scalable for high-velocity graph streams

---

## Datasets

Experiments are conducted on the following benchmark intrusion detection datasets:

- [DARPA](https://www.ll.mit.edu/r-d/datasets/1999-darpa-intrusion-detection-evaluation-dataset)
- [ISCX-IDS2012](https://www.unb.ca/cic/datasets/ids.html)
- [CIC-IDS2018](https://www.unb.ca/cic/datasets/ids-2018.html)
- [CIC-DDoS2019](https://www.unb.ca/cic/datasets/ddos-2019.html)

> Note: Datasets larger than 100MB (e.g., CIC-* datasets) are not uploaded to GitHub. Please download them from the official links above and place them inside the `data/` directory.

---

## Quick Start

### Requirements

- C++ (if using native sketch)
- Python 3.8+
- Packages:
  - `pandas`, `matplotlib`, `scikit-learn`, `numpy`

### To run on DARPA dataset:

```bash
bash demo.sh DARPA
```
### To run on ISCX dataset:

```bash
bash demo.sh ISCX
```

## Environment

This implementation has been tested on:

- **CPU:** 11th Gen Intel Core i5 @ 2.4GHz  
- **RAM:** 16 GB  
- **Operating System:** Windows 11 (64-bit)

---

## Results Summary

Our algorithm consistently outperforms existing methods such as **MIDAS**, **AnoEdge**, and **F-FADE** across multiple datasets in terms of **AUC-ROC** and **runtime**. 
