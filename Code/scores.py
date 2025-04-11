import pandas as pd
from sklearn.metrics import roc_auc_score
import warnings
warnings.filterwarnings("ignore", category=DeprecationWarning)

# Load the scores and labels
df = pd.read_csv("scores_labels.csv")

# Compute the correct AUC score
auc = roc_auc_score(df["label"], df["score"])
print(f"Correct AUC: {auc:.3f}")
