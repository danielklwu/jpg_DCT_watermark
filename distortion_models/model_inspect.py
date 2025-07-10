import pickle
import torch

def inspect_file(path):
    print(f"\nInspecting {path}")
    try:
        with open(path, 'rb') as f:
            obj = pickle.load(f)
        print("✅ Loaded with pickle.load()")
        print("Type:", type(obj))
        print("Content:", obj if isinstance(obj, (int, float, str, list, dict)) else "(not printed)")
    except Exception as e:
        print("❌ pickle.load() failed:", e)

    try:
        obj = torch.load(path, map_location='cpu')
        print("✅ Loaded with torch.load()")
        print("Type:", type(obj))
        if isinstance(obj, dict):
            print("Dict keys:", obj.keys())
    except Exception as e:
        print("❌ torch.load() failed:", e)

def inspect_keys(path):
    model_dict = torch.load(path, map_location='cpu')
    print(list(model_dict.keys()))
    
# Example usage
# inspect_file("geoProjModels/model_en.pkl")
# inspect_file("geoProjModels/model_de.pkl")
# inspect_file("geoProjModels/model_class.pkl")
inspect_keys("geoProjModels/model_en.pkl")
inspect_keys("geoProjModels/model_de.pkl")
inspect_keys("geoProjModels/model_class.pkl")