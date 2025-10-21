#!/usr/bin/env python3
"""
KrishiRakshak - Synthetic Farm Data Generator
Generates realistic sensor data for model training and testing
"""

import numpy as np
import pandas as pd
from datetime import datetime, timedelta
import json

class FarmDataGenerator:
    def __init__(self, seed=42):
        self.rng = np.random.default_rng(seed)
        self.class_names = ['Normal', 'WaterStress', 'PestRisk']
        
    def generate_normal_conditions(self, num_samples):
        """Generate data for normal, healthy crop conditions"""
        data = {
            'soil_moisture': self.rng.normal(50, 8, num_samples),
            'temperature': self.rng.normal(25, 2, num_samples),
            'humidity': self.rng.normal(65, 10, num_samples),
            'audio_energy': self.rng.uniform(0, 0.3, num_samples)
        }
        return pd.DataFrame(data), np.zeros(num_samples, dtype=int)
    
    def generate_water_stress(self, num_samples):
        """Generate data for water stress conditions"""
        data = {
            'soil_moisture': self.rng.normal(25, 6, num_samples),
            'temperature': self.rng.normal(35, 3, num_samples),
            'humidity': self.rng.normal(35, 8, num_samples),
            'audio_energy': self.rng.uniform(0.1, 0.4, num_samples)
        }
        return pd.DataFrame(data), np.ones(num_samples, dtype=int)
    
    def generate_pest_risk(self, num_samples):
        """Generate data for pest risk conditions"""
        data = {
            'soil_moisture': self.rng.normal(45, 12, num_samples),
            'temperature': self.rng.normal(28, 4, num_samples),
            'humidity': self.rng.normal(55, 15, num_samples),
            'audio_energy': self.rng.uniform(0.6, 0.9, num_samples)
        }
        return pd.DataFrame(data), np.ones(num_samples, dtype=int) * 2
    
    def generate_complete_dataset(self, samples_per_class=1000):
        """Generate complete balanced dataset"""
        print(" Generating synthetic farm dataset...")
        
        # Generate data for each class
        df_normal, labels_normal = self.generate_normal_conditions(samples_per_class)
        df_water, labels_water = self.generate_water_stress(samples_per_class)
        df_pest, labels_pest = self.generate_pest_risk(samples_per_class)
        
        # Combine all data
        df = pd.concat([df_normal, df_water, df_pest], ignore_index=True)
        labels = np.concatenate([labels_normal, labels_water, labels_pest])
        
        # Add some realistic constraints
        df['soil_moisture'] = np.clip(df['soil_moisture'], 0, 100)
        df['temperature'] = np.clip(df['temperature'], 10, 50)
        df['humidity'] = np.clip(df['humidity'], 0, 100)
        df['audio_energy'] = np.clip(df['audio_energy'], 0, 1)
        
        # Add timestamp for time series data
        base_time = datetime.now()
        timestamps = [base_time - timedelta(hours=i) for i in range(len(df))]
        df['timestamp'] = timestamps
        
        print(f" Generated {len(df)} samples")
        print(f"   Class distribution: Normal={np.sum(labels==0)}, "
              f"WaterStress={np.sum(labels==1)}, PestRisk={np.sum(labels==2)}")
        
        return df, labels
    
    def generate_real_time_samples(self, num_samples=10):
        """Generate real-time samples for simulation"""
        conditions = []
        
        for i in range(num_samples):
            # Randomly choose a condition to simulate
            condition_type = self.rng.choice([0, 1, 2], p=[0.6, 0.2, 0.2])
            
            if condition_type == 0:  # Normal
                sample = {
                    'soil_moisture': self.rng.normal(50, 5),
                    'temperature': self.rng.normal(25, 1.5),
                    'humidity': self.rng.normal(65, 5),
                    'audio_energy': self.rng.uniform(0, 0.2)
                }
            elif condition_type == 1:  # Water stress
                sample = {
                    'soil_moisture': self.rng.normal(20, 4),
                    'temperature': self.rng.normal(35, 2),
                    'humidity': self.rng.normal(30, 4),
                    'audio_energy': self.rng.uniform(0.1, 0.3)
                }
            else:  # Pest risk
                sample = {
                    'soil_moisture': self.rng.normal(45, 6),
                    'temperature': self.rng.normal(28, 2),
                    'humidity': self.rng.normal(55, 8),
                    'audio_energy': self.rng.uniform(0.7, 0.9)
                }
            
            # Apply constraints
            sample['soil_moisture'] = max(0, min(100, sample['soil_moisture']))
            sample['temperature'] = max(10, min(50, sample['temperature']))
            sample['humidity'] = max(0, min(100, sample['humidity']))
            sample['audio_energy'] = max(0, min(1, sample['audio_energy']))
            sample['condition'] = condition_type
            
            conditions.append(sample)
        
        return conditions
    
    def save_dataset(self, df, labels, filename='synthetic_farm_data.csv'):
        """Save dataset to CSV file"""
        df['label'] = labels
        df['condition'] = df['label'].map({0: 'Normal', 1: 'WaterStress', 2: 'PestRisk'})
        
        df.to_csv(filename, index=False)
        print(f" Dataset saved to {filename}")
        
        # Also save as JSON for web dashboard
        json_data = df.to_dict('records')
        with open(filename.replace('.csv', '.json'), 'w') as f:
            json.dump(json_data, f, indent=2, default=str)
        
        return filename

def main():
    """Main function to generate and save sample dataset"""
    generator = FarmDataGenerator(seed=42)
    
    # Generate complete dataset
    df, labels = generator.generate_complete_dataset(samples_per_class=1000)
    
    # Save to file
    filename = generator.save_dataset(df, labels)
    
    # Print sample statistics
    print("\n Dataset Statistics:")
    print(df.groupby('condition').agg({
        'soil_moisture': ['mean', 'std'],
        'temperature': ['mean', 'std'],
        'humidity': ['mean', 'std'],
        'audio_energy': ['mean', 'std']
    }).round(2))
    
    # Generate some real-time test samples
    print("\n Real-time Test Samples:")
    test_samples = generator.generate_real_time_samples(5)
    for i, sample in enumerate(test_samples):
        print(f"Sample {i+1}: {sample}")

if __name__ == "__main__":
    main()