# k-Stable Matching Simulations

This project implements simulations to test the computational complexity findings from the paper "Computational Complexity of k-stable Matchings" by Aziz, Csáji, and Cseh (DOI: 10.1145/3708507).

## Overview

The paper investigates k-stability in three matching market models:
- **House Allocation**: Assigning indivisible goods (houses) to agents
- **Marriage**: Two distinct sets of agents seeking to form pairs
- **Roommates**: A single set of agents looking to be paired among themselves

A matching is **k-stable** if no alternative matching exists that benefits at least k out of n agents more than the current matching.

## Experimental Design

### 1. Verification Complexity Testing
- **Hypothesis**: Verification should run in polynomial time O(n^c)
- **Method**: Measure execution time for different problem sizes
- **Expected Result**: Time should grow polynomially, not exponentially

### 2. Existence Complexity Testing
- **Hypothesis**: Complexity varies with k/n ratio
- **Method**: Test different k/n ratios and measure execution time
- **Expected Result**: Different complexity patterns for different ratios

### 3. Model Comparison
- **Hypothesis**: Different models may have different complexity characteristics
- **Method**: Compare execution times across house allocation, marriage, and roommates models
- **Expected Result**: Model-specific complexity differences

### 4. k/n Ratio Analysis
- **Hypothesis**: Existence probability and complexity depend on k/n ratio
- **Method**: Test various k/n ratios and measure both existence rate and computation time
- **Expected Result**: Clear relationship between ratio and both existence and complexity

## Key Algorithms Implemented

### k-Stability Verification
- **Algorithm**: Check for blocking coalitions of size ≥ k
- **Complexity**: Polynomial time (as claimed in paper)
- **Implementation**: `is_k_stable_direct()` in `verification.c`

### k-Stable Matching Existence
- **Algorithm**: Recursive backtracking with pruning
- **Complexity**: Depends on k/n ratio (as claimed in paper)
- **Implementation**: `k_stable_matching_exists()` in `existence.c`

### Test Case Generation
- **House Allocation**: Random preference lists for each agent
- **Marriage**: Separate preference lists for men and women
- **Roommates**: Preference lists excluding self-preference

## Analysis Tools

The project includes several analysis functions:

- `benchmark_verification_complexity()`: Tests polynomial time claim
- `benchmark_existence_complexity()`: Tests k/n ratio effects
- `benchmark_model_comparison()`: Compares different matching models
- `analyze_k_ratio_effect()`: Analyzes relationship between k/n and existence

## References

- Aziz, H., Csáji, G., & Cseh, Á. (2025). Computational Complexity of k-stable Matchings. ACM Transactions on Economics and Computation, 13(1).
- DOI: 10.1145/3708507
- arXiv preprint: 2307.03794
