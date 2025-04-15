import zipfile
import sys
import os

def extract_dataset(dataset_name):
    zip_path = "../data/Large Datasets.zip"
    base_dir = f"AnoGraph Datasets/{dataset_name.upper()}"
    extract_to = f"./tmp_data"

    os.makedirs(extract_to, exist_ok=True)

    file_map = {
        "data": f"{base_dir}/Data.csv",
        "label": f"{base_dir}/Label.csv"
    }

    with zipfile.ZipFile(zip_path, 'r') as zipf:
        for key, internal_path in file_map.items():
            if internal_path not in zipf.namelist():
                print(f"File not found in zip: {internal_path}")
                sys.exit(1)

            output_file = os.path.join(extract_to, f"{dataset_name}_" + ("Data.csv" if key == "data" else "Label.csv"))
            with zipf.open(internal_path) as src, open(output_file, 'wb') as dst:
                dst.write(src.read())

    print(f"Extracted files to: {extract_to}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 extract_from_zip.py <dataset_name>")
        sys.exit(1)

    dataset = sys.argv[1].lower()
    if dataset not in ["ids2018", "ddos2019"]:
        print("Only 'ids2018' and 'ddos2019' are supported via ZIP.")
        sys.exit(1)

    extract_dataset(dataset)
