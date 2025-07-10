import cv2
import numpy as np
import torch
import torch.nn as nn
import torchvision.transforms as transforms
from PIL import Image
import argparse
import os

class DistortionCorrector:
    def __init__(self, model_dir, device='cpu'):
        """
        Initialize the distortion corrector.
        
        Args:
            model_dir (str): Directory containing the .pkl model files
            device (str): Device to run inference on ('cpu' or 'cuda')
        """
        self.device = torch.device(device if torch.cuda.is_available() else 'cpu')
        self.model_dir = model_dir
        
        # Load the three model components
        self.encoder_model = self.load_model(os.path.join(model_dir, 'model_en.pkl'))
        self.decoder_model = self.load_model(os.path.join(model_dir, 'model_de.pkl'))
        self.classifier_model = self.load_model(os.path.join(model_dir, 'model_class.pkl'))
        
        print(f"Loaded models from {model_dir}")
        print(f"Encoder type: {type(self.encoder_model)}")
        print(f"Decoder type: {type(self.decoder_model)}")
        print(f"Classifier type: {type(self.classifier_model)}")
        
        # Set models to evaluation mode if they are PyTorch models
        self.set_eval_mode()
        
        # Define image transformations (may need adjustment based on your specific model)
        self.transform = transforms.Compose([
            transforms.Resize((256, 256)),  # Adjust size based on your model
            transforms.ToTensor(),
        ])
    
    def load_model(self, model_path):
        """Load a PyTorch model from file using torch.load only"""
        if not os.path.exists(model_path):
            raise FileNotFoundError(f"Model file not found: {model_path}")
        try:
            model = torch.load(model_path, map_location=self.device)
            return model
        except Exception as e:
            raise Exception(f"Failed to load model {model_path} with torch.load. Error: {e}")
    
    def set_eval_mode(self):
        """Set PyTorch models to evaluation mode"""
        for model in [self.encoder_model, self.decoder_model, self.classifier_model]:
            if hasattr(model, 'eval'):
                model.eval()
            if hasattr(model, 'to'):
                model.to(self.device)
    
    def preprocess_image(self, image_path):
        """
        Preprocess the input image for the model.
        
        Args:
            image_path (str): Path to the input image
            
        Returns:
            Various formats depending on model type
        """
        # Load image
        image = Image.open(image_path).convert('RGB')
        original_size = image.size
        
        # Convert to numpy array for traditional ML models
        image_np = np.array(image)
        
        # Apply transformations for deep learning models
        image_tensor = self.transform(image).unsqueeze(0)
        
        return {
            'pil': image,
            'numpy': image_np,
            'tensor': image_tensor.to(self.device),
            'original_size': original_size
        }
    
    def extract_features(self, image_data):
        """
        Extract features from the image using the encoder model.
        This method adapts to different model types.
        """
        if hasattr(self.encoder_model, 'predict'):
            # Scikit-learn style model
            if image_data['numpy'].ndim == 3:
                # Flatten the image for traditional ML models
                features = image_data['numpy'].flatten().reshape(1, -1)
            else:
                features = image_data['numpy']
            return self.encoder_model.predict(features)
        
        elif hasattr(self.encoder_model, 'forward') or callable(self.encoder_model):
            # PyTorch model
            with torch.no_grad():
                return self.encoder_model(image_data['tensor'])
        
        else:
            # Try to call the model directly
            try:
                return self.encoder_model(image_data['tensor'])
            except:
                # Fallback: return the original tensor
                return image_data['tensor']
    
    def classify_distortion(self, features):
        """
        Classify the type/parameters of distortion using the classifier model.
        """
        if hasattr(self.classifier_model, 'predict'):
            # Scikit-learn style model
            if hasattr(features, 'cpu'):
                features = features.cpu().numpy()
            if features.ndim > 2:
                features = features.reshape(features.shape[0], -1)
            return self.classifier_model.predict(features)
        
        elif hasattr(self.classifier_model, 'forward') or callable(self.classifier_model):
            # PyTorch model
            with torch.no_grad():
                return self.classifier_model(features)
        
        else:
            # Return features as-is if classifier can't be applied
            return features
    
    def decode_correction(self, encoded_features, distortion_params):
        """
        Decode the corrected image using the decoder model.
        """
        if hasattr(self.decoder_model, 'predict'):
            # Scikit-learn style model
            input_data = encoded_features
            if hasattr(distortion_params, 'cpu'):
                distortion_params = distortion_params.cpu().numpy()
            if hasattr(input_data, 'cpu'):
                input_data = input_data.cpu().numpy()
            
            # Combine features if needed
            if input_data.ndim > 2:
                input_data = input_data.reshape(input_data.shape[0], -1)
            
            corrected = self.decoder_model.predict(input_data)
            return corrected
        
        elif hasattr(self.decoder_model, 'forward') or callable(self.decoder_model):
            # PyTorch model
            with torch.no_grad():
                return self.decoder_model(encoded_features)
        
        else:
            # Return encoded features as-is if decoder can't be applied
            return encoded_features
    
    def postprocess_image(self, output_data, original_size):
        """
        Postprocess the model output to get the final image.
        
        Args:
            output_data: Model output (could be tensor, numpy array, etc.)
            original_size (tuple): Original image dimensions
            
        Returns:
            PIL.Image: Corrected image
        """
        try:
            # Convert to numpy array if it's a tensor
            if hasattr(output_data, 'cpu'):
                output_np = output_data.cpu().numpy()
            else:
                output_np = np.array(output_data)
            
            # Handle different output shapes
            if output_np.ndim == 4:  # Batch dimension
                output_np = output_np.squeeze(0)
            
            if output_np.ndim == 3 and output_np.shape[0] == 3:  # CHW format
                output_np = output_np.transpose(1, 2, 0)  # Convert to HWC
            
            # Ensure values are in [0, 1] range
            if output_np.max() <= 1.0:
                output_np = (output_np * 255).astype(np.uint8)
            else:
                output_np = np.clip(output_np, 0, 255).astype(np.uint8)
            
            # Handle grayscale to RGB conversion if needed
            if output_np.ndim == 2:
                output_np = np.stack([output_np] * 3, axis=2)
            
            # Convert to PIL Image
            corrected_image = Image.fromarray(output_np)
            
            # Resize to original dimensions
            corrected_image = corrected_image.resize(original_size, Image.LANCZOS)
            
            return corrected_image
            
        except Exception as e:
            print(f"Error in postprocessing: {e}")
            # Return a placeholder image if processing fails
            return Image.new('RGB', original_size, (128, 128, 128))
    
    def correct_distortion(self, input_path, output_path):
        """
        Correct geometric distortion in an image using the three-model pipeline.
        
        Args:
            input_path (str): Path to the input image
            output_path (str): Path to save the corrected image
        """
        try:
            # Preprocess the image
            image_data = self.preprocess_image(input_path)
            print("Image preprocessed successfully")
            
            # Step 1: Extract features using encoder
            encoded_features = self.extract_features(image_data)
            print(f"Features extracted: {type(encoded_features)}")
            
            # Step 2: Classify distortion parameters
            distortion_params = self.classify_distortion(encoded_features)
            print(f"Distortion classified: {type(distortion_params)}")
            
            # Step 3: Decode corrected image
            corrected_output = self.decode_correction(encoded_features, distortion_params)
            print(f"Correction decoded: {type(corrected_output)}")
            
            # Postprocess to get final image
            corrected_image = self.postprocess_image(corrected_output, image_data['original_size'])
            
            # Save the result
            corrected_image.save(output_path, 'JPEG', quality=95)
            print(f"Corrected image saved to {output_path}")
            
        except Exception as e:
            print(f"Error processing image: {str(e)}")
            import traceback
            traceback.print_exc()

def main():
    parser = argparse.ArgumentParser(description='Blind Geometric Distortion Correction')
    parser.add_argument('--input', '-i', required=True, help='Path to input JPG image')
    parser.add_argument('--output', '-o', required=True, help='Path to output JPG image')
    parser.add_argument('--model_dir', '-m', required=True, help='Directory containing model_en.pkl, model_de.pkl, and model_class.pkl')
    parser.add_argument('--device', '-d', default='cpu', choices=['cpu', 'cuda'], 
                       help='Device to run inference on')
    
    args = parser.parse_args()
    
    # Validate input file
    if not os.path.exists(args.input):
        print(f"Error: Input file {args.input} does not exist")
        return
    
    if not args.input.lower().endswith(('.jpg', '.jpeg')):
        print("Warning: Input file should be a JPG image")
    
    # Validate model directory
    if not os.path.exists(args.model_dir):
        print(f"Error: Model directory {args.model_dir} does not exist")
        return
    
    required_files = ['model_en.pkl', 'model_de.pkl', 'model_class.pkl']
    missing_files = []
    for file in required_files:
        if not os.path.exists(os.path.join(args.model_dir, file)):
            missing_files.append(file)
    
    if missing_files:
        print(f"Error: Missing model files: {missing_files}")
        return
    
    # Create output directory if it doesn't exist
    output_dir = os.path.dirname(args.output)
    if output_dir and not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    # Initialize corrector and process image
    corrector = DistortionCorrector(args.model_dir, args.device)
    corrector.correct_distortion(args.input, args.output)

if __name__ == "__main__":
    main()