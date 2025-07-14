# JPEG DCT Watermarking & Distortion Correction

This project implements a robust digital watermarking system for JPEG images using Discrete Cosine Transform (DCT) techniques, with support for blind geometric distortion correction using deep learning. The system is designed to embed, extract, and recover watermarks from images subjected to real-world attacks such as rotation, scaling, and compression.

## Features

- **DCT-Based Watermark Embedding and Extraction:**  
  Embed and extract binary watermarks in JPEG images using block-based DCT manipulation (C implementation).

- **Support for Multiple Image Formats:**  
  Convert and process various image formats (JPEG, PNG, etc.) using ImageMagick.

- **Robustness to Attacks:**  
  Simulate and test watermark resilience against noise, compression, and geometric distortions.

- **Blind Geometric Distortion Correction:**  
  Python pipeline leveraging pre-trained PyTorch models to automatically correct geometric distortions before watermark extraction.

- **Batch Processing & Automation:**  
  Easily process multiple images and automate testing of watermark robustness.

## Directory Structure

```
jpg_DCT_watermark/
├── src/                  # C source files for watermarking and attacks
├── inc/                  # C header files
├── distorted_inputs/     # Example distorted images
├── corrected_outputs/    # Output images after correction
├── geoProjModels/        # Pre-trained PyTorch model files (.pkl)
├── distortion_models/    # Python scripts for distortion detection/correction
├── distortion_correction.py # Main Python script for distortion correction
├── Makefile              # Build instructions for C code
├── README.md             # Project documentation
```

## Requirements

### C Components
- GCC or compatible C compiler
- [libjpeg](http://libjpeg.sourceforge.net/)
- [ImageMagick (MagickWand)](https://imagemagick.org/)

### Python Components
- Python 3.7+
- `torch`, `torchvision`
- `numpy`
- `opencv-python`
- `Pillow`

Install Python dependencies with:
```bash
pip install torch torchvision numpy opencv-python Pillow
```

## Building the C Code

```bash
make
```

## Usage

### 1. Watermark Embedding & Extraction (C)

Embed a watermark and test robustness:
```bash
./main input1.jpg
```
- Outputs: `watermarked_image.jpg`, `noisy_watermarked_image.jpg`, `jpeg_compressed_watermarked.jpg`
- Prints similarity statistics for watermark recovery after attacks.

### 2. Blind Distortion Correction (Python)

Correct geometric distortions using pre-trained models:
```bash
python distortion_correction.py \
  --input distorted_inputs/rotation.jpg \
  --output corrected_outputs/rotation_corrected.jpg \
  --model_dir geoProjModels/ \
  --device cpu
```

### 3. End-to-End Workflow

1. Embed watermark using the C tool.
2. Apply distortions (manually or using provided scripts).
3. Correct distortions with `distortion_correction.py`.
4. Extract watermark from the corrected image.

## Pre-trained Models

Place the following files in the `geoProjModels/` directory:
- `model_en.pkl` (Encoder)
- `model_de.pkl` (Decoder)
- `model_class.pkl` (Classifier)

## Example

```bash
# Embed watermark
./main input1.jpg

# Correct distortion
python distortion_correction.py \
  --input distorted_inputs/rotation.jpg \
  --output corrected_outputs/rotation_corrected.jpg \
  --model_dir geoProjModels/

# Extract watermark (C tool, using corrected image)
./main corrected_outputs/rotation_corrected.jpg
```

## Acknowledgements

- DCT watermarking techniques inspired by academic literature.
- Deep learning models trained for geometric correction using PyTorch.

## License

This project is for educational and research purposes. See [LICENSE](LICENSE) for details.
