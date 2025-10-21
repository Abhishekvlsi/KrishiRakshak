#!/usr/bin/env python3
"""
KrishiRakshak - Edge AI Simulator
Simulates the complete edge inference pipeline for testing
"""

import numpy as np
import tensorflow as tf
import time
import json
from datetime import datetime
from data_generator import FarmDataGenerator

class EdgeInferenceSimulator:
    def __init__(self, model_path=None):
        self.generator = FarmDataGenerator()
        self.model = None
        self.interpreter = None
        self.inference_times = []
        
        if model_path:
            self.load_model(model_path)
        else:
            self.create_dummy_model()
    
    def create_dummy_model(self):
        """Create a simple model for simulation"""
        print("🤖 Creating dummy ML model for simulation...")
        
        self.model = tf.keras.Sequential([
            tf.keras.layers.Dense(64, activation='relu', input_shape=(4,)),
            tf.keras.layers.Dropout(0.2),
            tf.keras.layers.Dense(32, activation='relu'),
            tf.keras.layers.Dense(16, activation='relu'),
            tf.keras.layers.Dense(3, activation='softmax')
        ])
        
        self.model.compile(optimizer='adam', 
                          loss='sparse_categorical_crossentropy', 
                          metrics=['accuracy'])
        
        # Generate some dummy data for demonstration
        X, y = self.generator.generate_complete_dataset(100)
        self.model.fit(X[['soil_moisture', 'temperature', 'humidity', 'audio_energy']].values, 
                      y, epochs=10, verbose=0)
        
        print(" Dummy model created and trained")
    
    def load_model(self, model_path):
        """Load a pre-trained TensorFlow model"""
        print(f" Loading model from {model_path}...")
        self.model = tf.keras.models.load_model(model_path)
        print(" Model loaded successfully")
    
    def preprocess_sensor_data(self, sensor_data):
        """Preprocess sensor data for ML inference"""
        features = np.array([
            sensor_data['soil_moisture'] / 100.0,      # Normalize to 0-1
            (sensor_data['temperature'] - 10) / 40.0,  # Normalize 10-50°C to 0-1
            sensor_data['humidity'] / 100.0,           # Normalize to 0-1
            sensor_data['audio_energy']                # Already 0-1
        ]).reshape(1, -1)
        
        return features
    
    def run_inference(self, sensor_data):
        """Run ML inference on sensor data"""
        if self.model is None:
            raise ValueError("Model not loaded")
        
        # Preprocess data
        features = self.preprocess_sensor_data(sensor_data)
        
        # Run inference with timing
        start_time = time.time()
        predictions = self.model.predict(features, verbose=0)
        inference_time = (time.time() - start_time) * 1000  # Convert to ms
        
        self.inference_times.append(inference_time)
        
        # Get results
        predicted_class = np.argmax(predictions[0])
        confidence = predictions[0][predicted_class]
        
        result = {
            'predicted_class': int(predicted_class),
            'confidence': float(confidence),
            'inference_time_ms': float(inference_time),
            'raw_predictions': predictions[0].tolist(),
            'timestamp': datetime.now().isoformat()
        }
        
        return result
    
    def simulate_real_time_operation(self, duration_minutes=10, sample_interval_sec=30):
        """Simulate real-time edge device operation"""
        print(f" Starting real-time simulation for {duration_minutes} minutes...")
        print(f"   Sample interval: {sample_interval_sec} seconds")
        print("-" * 60)
        
        total_samples = (duration_minutes * 60) // sample_interval_sec
        alerts_sent = 0
        
        for i in range(total_samples):
            # Generate realistic sensor data
            sensor_sample = self.generator.generate_real_time_samples(1)[0]
            
            # Run inference
            result = self.run_inference(sensor_sample)
            
            # Decision logic (simulating edge device behavior)
            confidence_threshold = 0.7
            if result['confidence'] > confidence_threshold:
                alert_type = ['Normal', 'Water Stress', 'Pest Risk'][result['predicted_class']]
                if result['predicted_class'] != 0:  # Don't alert for normal conditions
                    alerts_sent += 1
                    alert_status = f" ALERT: {alert_type}"
                else:
                    alert_status = " Normal"
            else:
                alert_status
