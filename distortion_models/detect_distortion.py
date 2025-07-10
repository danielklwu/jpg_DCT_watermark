import torch
import torch.nn as nn
import torch.nn.functional as F
import torchvision.transforms as transforms
from PIL import Image
import numpy as np
import argparse
import os

# Import the model architectures from the original files
from modelNetM import EncoderNet as EncoderNetM, ClassNet
from modelNetS import EncoderNet as EncoderNetS

class DistortionDetector:
    def __init__(self, model_paths=None, device='cuda' if torch.cuda.is_available() else 'cpu'):
        """
        Initialize the distortion detector
        
        Args:
            model_paths: Dictionary containing paths to pretrained models
                        {'encoder': 'path/to/encoder.pth', 
                         'decoder': 'path/to/decoder.pth',
                         'classifier': 'path/to/classifier.pth'}
            device: Device to run inference on ('cuda' or 'cpu')
        """
        self.device = device
        self.distortion_types = [
            'barrel', 'pincushion', 'rotation', 
            'shear', 'projective', 'wave'
        ]
        
        # Initialize models
        self.encoder_m = EncoderNetM([2, 2, 2, 2, 2]).to(device)
        self.encoder_s = EncoderNetS([2, 2, 2, 2, 2]).to(device)
        self.classifier = ClassNet().to(device)
        
        # Load pretrained weights if provided
        if model_paths:
            self.load_models(model_paths)
        
        # Set models to evaluation mode
        self.encoder_m.eval()
        self.encoder_s.eval()
        self.classifier.eval()
        
        # Define image preprocessing
        self.transform = transforms.Compose([
            transforms.Resize((256, 256)),
            transforms.ToTensor(),
            transforms.Normalize(mean=[0.485, 0.456, 0.406], 
                               std=[0.229, 0.224, 0.225])
        ])
    
    def load_models(self, model_paths):
        """Load pretrained model weights"""
        try:
            if 'encoder' in model_paths and os.path.exists(model_paths['encoder']):
                self.encoder_m.load_state_dict(torch.load(model_paths['encoder'], map_location=self.device))
                print(f"Loaded encoder model from {model_paths['encoder']}")
            
            if 'classifier' in model_paths and os.path.exists(model_paths['classifier']):
                self.classifier.load_state_dict(torch.load(model_paths['classifier'], map_location=self.device))
                print(f"Loaded classifier model from {model_paths['classifier']}")
                
        except Exception as e:
            print(f"Warning: Could not load pretrained models: {e}")
            print("Using randomly initialized weights.")
    
    def preprocess_image(self, image_path):
        """
        Preprocess input image for inference
        
        Args:
            image_path: Path to input image
            
        Returns:
            Preprocessed image tensor
        """
        try:
            # Load and convert image
            image = Image.open(image_path).convert('RGB')
            
            # Apply transformations
            image_tensor = self.transform(image).unsqueeze(0)  # Add batch dimension
            
            return image_tensor.to(self.device)
            
        except Exception as e:
            raise ValueError(f"Error preprocessing image: {e}")
    
    def detect_distortion(self, image_path, return_probabilities=False):
        """
        Detect the type of geometric distortion in an image
        
        Args:
            image_path: Path to input image
            return_probabilities: If True, return probability distribution
            
        Returns:
            Dictionary containing detection results
        """
        with torch.no_grad():
            # Preprocess image
            image_tensor = self.preprocess_image(image_path)
            
            # Extract features using encoder
            features = self.encoder_m(image_tensor)
            
            # Classify distortion type
            logits = self.classifier(features)
            probabilities = F.softmax(logits, dim=1)
            
            # Get prediction
            predicted_class = torch.argmax(probabilities, dim=1).item()
            confidence = probabilities[0, predicted_class].item()
            
            results = {
                'distortion_type': self.distortion_types[predicted_class],
                'confidence': confidence,
                'predicted_class_index': predicted_class
            }
            
            if return_probabilities:
                prob_dict = {
                    distortion_type: prob.item() 
                    for distortion_type, prob in zip(self.distortion_types, probabilities[0])
                }
                results['all_probabilities'] = prob_dict
            
            return results
    
    def batch_detect(self, image_paths, return_probabilities=False):
        """
        Detect distortion types for multiple images
        
        Args:
            image_paths: List of paths to input images
            return_probabilities: If True, return probability distributions
            
        Returns:
            List of detection results for each image
        """
        results = []
        
        for image_path in image_paths:
            try:
                result = self.detect_distortion(image_path, return_probabilities)
                result['image_path'] = image_path
                results.append(result)
                
            except Exception as e:
                results.append({
                    'image_path': image_path,
                    'error': str(e)
                })
        
        return results
    
    def estimate_parameters(self, image_path, distortion_type):
        """
        Estimate distortion parameters for a specific distortion type
        
        Args:
            image_path: Path to input image
            distortion_type: Type of distortion to estimate parameters for
            
        Returns:
            Estimated parameter value
        """
        with torch.no_grad():
            # Preprocess image
            image_tensor = self.preprocess_image(image_path)
            
            # Use specialized encoder for parameter estimation
            parameter = self.encoder_s(image_tensor)
            
            return parameter.item()

def main():
    parser = argparse.ArgumentParser(description='Detect geometric distortion in images')
    parser.add_argument('--image', required=True, help='Path to input image')
    parser.add_argument('--model_dir', default='geoProjModels', help='Directory containing pretrained models')
    parser.add_argument('--batch', nargs='+', help='Multiple image paths for batch processing')
    parser.add_argument('--probabilities', action='store_true', 
                       help='Show probability distribution for all distortion types')
    parser.add_argument('--estimate_params', action='store_true',
                       help='Estimate distortion parameters')
    parser.add_argument('--device', default='auto', choices=['auto', 'cuda', 'cpu'],
                       help='Device to use for inference')
    
    args = parser.parse_args()
    
    # Set device
    if args.device == 'auto':
        device = 'cuda' if torch.cuda.is_available() else 'cpu'
    else:
        device = args.device
    
    # Prepare model paths
    model_paths = {}
    if args.model_dir:
        model_paths = {
            'encoder': os.path.join(args.model_dir, 'model_en.pth'),
            'classifier': os.path.join(args.model_dir, 'model_class.pth')
        }
    
    # Initialize detector
    detector = DistortionDetector(model_paths, device)
    
    print(f"Using device: {device}")
    print("-" * 50)
    
    # Process images
    if args.batch:
        # Batch processing
        results = detector.batch_detect(args.batch, args.probabilities)
        
        for result in results:
            if 'error' in result:
                print(f"Error processing {result['image_path']}: {result['error']}")
            else:
                print(f"Image: {result['image_path']}")
                print(f"Detected distortion: {result['distortion_type']}")
                print(f"Confidence: {result['confidence']:.4f}")
                
                if args.probabilities:
                    print("All probabilities:")
                    for dist_type, prob in result['all_probabilities'].items():
                        print(f"  {dist_type}: {prob:.4f}")
                
                if args.estimate_params:
                    param = detector.estimate_parameters(result['image_path'], 
                                                       result['distortion_type'])
                    print(f"Estimated parameter: {param:.6f}")
                
                print("-" * 30)
    
    else:
        # Single image processing
        try:
            result = detector.detect_distortion(args.image, args.probabilities)
            
            print(f"Image: {args.image}")
            print(f"Detected distortion: {result['distortion_type']}")
            print(f"Confidence: {result['confidence']:.4f}")
            
            if args.probabilities:
                print("\nAll probabilities:")
                for dist_type, prob in result['all_probabilities'].items():
                    print(f"  {dist_type}: {prob:.4f}")
            
            if args.estimate_params:
                param = detector.estimate_parameters(args.image, result['distortion_type'])
                print(f"\nEstimated parameter: {param:.6f}")
                
        except Exception as e:
            print(f"Error processing image: {e}")

if __name__ == "__main__":
    main()