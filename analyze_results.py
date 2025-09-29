#!/usr/bin/env python3
"""
Analysis script for k-stable matching simulation results.
This script helps visualize and analyze the computational complexity results.
"""

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import argparse
import sys

def parse_benchmark_output(filename):
    """Parse benchmark output from the C program."""
    results = {
        'verification': [],
        'existence': [],
        'model_comparison': []
    }
    
    try:
        with open(filename, 'r') as f:
            lines = f.readlines()
    except FileNotFoundError:
        print(f"Error: Could not find file {filename}")
        return results
    
    current_section = None
    
    for line in lines:
        line = line.strip()
        
        if "Benchmarking k-Stability Verification Complexity" in line:
            current_section = 'verification'
            continue
        elif "Benchmarking k-Stable Matching Existence Complexity" in line:
            current_section = 'existence'
            continue
        elif "Comparing Different Matching Models" in line:
            current_section = 'model_comparison'
            continue
        
        # Parse data lines
        if current_section and line and not line.startswith('=') and not line.startswith('Testing'):
            parts = line.split('\t')
            if len(parts) >= 3:
                try:
                    if current_section == 'verification':
                        agents = int(parts[0])
                        avg_time = float(parts[1])
                        std_dev = float(parts[2])
                        trials = int(parts[3])
                        results['verification'].append({
                            'agents': agents,
                            'avg_time': avg_time,
                            'std_dev': std_dev,
                            'trials': trials
                        })
                    elif current_section == 'existence':
                        agents = int(parts[0])
                        k_ratio = float(parts[1])
                        avg_time = float(parts[2])
                        std_dev = float(parts[3])
                        trials = int(parts[4])
                        exists_rate = float(parts[5])
                        results['existence'].append({
                            'agents': agents,
                            'k_ratio': k_ratio,
                            'avg_time': avg_time,
                            'std_dev': std_dev,
                            'trials': trials,
                            'exists_rate': exists_rate
                        })
                except (ValueError, IndexError):
                    continue
    
    return results

def plot_verification_complexity(results):
    """Plot verification complexity results."""
    if not results['verification']:
        print("No verification results to plot")
        return
    
    df = pd.DataFrame(results['verification'])
    
    plt.figure(figsize=(10, 6))
    
    # Plot actual times
    plt.errorbar(df['agents'], df['avg_time'], yerr=df['std_dev'], 
                marker='o', capsize=5, label='Actual Times')
    
    # Plot polynomial fits
    x = df['agents'].values
    y = df['avg_time'].values
    
    # Try different polynomial degrees
    for degree in [2, 3, 4]:
        coeffs = np.polyfit(x, y, degree)
        poly = np.poly1d(coeffs)
        x_fit = np.linspace(x.min(), x.max(), 100)
        y_fit = poly(x_fit)
        plt.plot(x_fit, y_fit, '--', alpha=0.7, 
                label=f'Polynomial degree {degree}')
    
    plt.xlabel('Number of Agents (n)')
    plt.ylabel('Average Time (ms)')
    plt.title('k-Stability Verification Complexity\n(Should be polynomial)')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig('verification_complexity.png', dpi=300, bbox_inches='tight')
    plt.show()

def plot_existence_complexity(results):
    """Plot existence complexity results."""
    if not results['existence']:
        print("No existence results to plot")
        return
    
    df = pd.DataFrame(results['existence'])
    
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 6))
    
    # Plot 1: Time vs k/n ratio
    for agents in df['agents'].unique():
        subset = df[df['agents'] == agents]
        ax1.errorbar(subset['k_ratio'], subset['avg_time'], 
                    yerr=subset['std_dev'], marker='o', 
                    label=f'n={agents}', capsize=3)
    
    ax1.set_xlabel('k/n Ratio')
    ax1.set_ylabel('Average Time (ms)')
    ax1.set_title('Existence Checking Time vs k/n Ratio')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # Plot 2: Existence rate vs k/n ratio
    for agents in df['agents'].unique():
        subset = df[df['agents'] == agents]
        ax2.plot(subset['k_ratio'], subset['exists_rate'], 
                marker='o', label=f'n={agents}')
    
    ax2.set_xlabel('k/n Ratio')
    ax2.set_ylabel('Existence Rate')
    ax2.set_title('k-Stable Matching Existence Rate vs k/n Ratio')
    ax2.legend()
    ax2.grid(True, alpha=0.3)
    ax2.set_ylim(0, 1)
    
    plt.tight_layout()
    plt.savefig('existence_analysis.png', dpi=300, bbox_inches='tight')
    plt.show()

def analyze_complexity_trends(results):
    """Analyze and report on complexity trends."""
    print("\n=== Complexity Analysis ===")
    
    # Verification complexity analysis
    if results['verification']:
        df_ver = pd.DataFrame(results['verification'])
        print(f"\nVerification Complexity:")
        print(f"  - Tested {len(df_ver)} different problem sizes")
        print(f"  - Range: {df_ver['agents'].min()} to {df_ver['agents'].max()} agents")
        print(f"  - Time range: {df_ver['avg_time'].min():.3f} to {df_ver['avg_time'].max():.3f} ms")
        
        # Check if growth is polynomial
        x = df_ver['agents'].values
        y = df_ver['avg_time'].values
        
        # Calculate growth rate
        if len(x) > 1:
            growth_rates = []
            for i in range(1, len(x)):
                rate = (y[i] - y[i-1]) / (x[i] - x[i-1])
                growth_rates.append(rate)
            
            avg_growth_rate = np.mean(growth_rates)
            print(f"  - Average growth rate: {avg_growth_rate:.3f} ms/agent")
            
            # Check for exponential growth (should not be exponential)
            if max(growth_rates) / min(growth_rates) > 10:
                print("  - WARNING: Growth rate varies significantly - may not be polynomial")
            else:
                print("  - Growth appears polynomial (good)")
    
    # Existence complexity analysis
    if results['existence']:
        df_ex = pd.DataFrame(results['existence'])
        print(f"\nExistence Complexity:")
        print(f"  - Tested {len(df_ex)} different configurations")
        print(f"  - k/n ratios tested: {sorted(df_ex['k_ratio'].unique())}")
        
        # Analyze relationship between k/n ratio and existence rate
        avg_existence_by_ratio = df_ex.groupby('k_ratio')['exists_rate'].mean()
        print(f"  - Average existence rates by k/n ratio:")
        for ratio, rate in avg_existence_by_ratio.items():
            print(f"    k/n = {ratio:.2f}: {rate:.3f}")
        
        # Analyze relationship between k/n ratio and computation time
        avg_time_by_ratio = df_ex.groupby('k_ratio')['avg_time'].mean()
        print(f"  - Average computation times by k/n ratio:")
        for ratio, time in avg_time_by_ratio.items():
            print(f"    k/n = {ratio:.2f}: {time:.3f} ms")

def main():
    parser = argparse.ArgumentParser(description='Analyze k-stable matching simulation results')
    parser.add_argument('input_file', help='Input file with benchmark results')
    parser.add_argument('--plot', action='store_true', help='Generate plots')
    parser.add_argument('--analyze', action='store_true', help='Print analysis')
    
    args = parser.parse_args()
    
    # Parse results
    results = parse_benchmark_output(args.input_file)
    
    if not any(results.values()):
        print("No results found in input file")
        return
    
    # Generate plots if requested
    if args.plot:
        print("Generating plots...")
        plot_verification_complexity(results)
        plot_existence_complexity(results)
        print("Plots saved as 'verification_complexity.png' and 'existence_analysis.png'")
    
    # Print analysis if requested
    if args.analyze:
        analyze_complexity_trends(results)
    
    # If no specific action requested, do both
    if not args.plot and not args.analyze:
        print("Generating plots and analysis...")
        plot_verification_complexity(results)
        plot_existence_complexity(results)
        analyze_complexity_trends(results)

if __name__ == '__main__':
    main()
