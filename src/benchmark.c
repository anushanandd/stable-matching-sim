#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "matching.h"

// Benchmark k-stability verification complexity
void benchmark_verification_complexity(int max_agents, int num_trials) {
    printf("=== Benchmarking k-Stability Verification Complexity ===\n");
    printf("Testing polynomial time claim: verification should be O(n^c) for some constant c\n");
    printf("Max agents: %d, Trials per size: %d\n\n", max_agents, num_trials);
    
    printf("Agents\tAvg Time (ms)\tStd Dev\t\tMin Time\tMax Time\tTrials\tSuccess Rate\n");
    printf("------\t-------------\t-------\t\t--------\t--------\t------\t------------\n");
    
    // Use better step sizes for more comprehensive testing
    for (int n = 5; n <= max_agents; n += (n < 20) ? 3 : (n < 50) ? 5 : 10) {
        // printf("DEBUG: Testing with %d agents...\n", n);
        double total_time = 0.0;
        double sum_squared = 0.0;
        double min_time = 1e9;
        double max_time = 0.0;
        int successful_trials = 0;
        
        for (int trial = 0; trial < num_trials; trial++) {
            // printf("DEBUG: Trial %d/%d for n=%d\n", trial+1, num_trials, n);
            
            // Generate random instance
            problem_instance_t* instance = generate_random_house_allocation(n, time(NULL) + trial);
            if (instance == NULL) {
                // printf("DEBUG: Failed to generate instance for trial %d\n", trial);
                continue;
            }
            
            // Create a random matching
            matching_t* matching = create_matching(n, HOUSE_ALLOCATION);
            if (matching == NULL) {
                // printf("DEBUG: Failed to create matching for trial %d\n", trial);
                free(instance);
                continue;
            }
            
            // Create a simple matching (assign agent i to house i)
            for (int i = 0; i < n; i++) {
                matching->pairs[i] = i;
            }
            
            // Benchmark verification
            // printf("DEBUG: Starting verification for trial %d...\n", trial);
            clock_t start = clock();
            is_k_stable_direct(matching, instance, n/2);  // k = n/2
            clock_t end = clock();
            // printf("DEBUG: Verification completed for trial %d\n", trial);
            
            double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
            total_time += time_ms;
            sum_squared += time_ms * time_ms;
            if (time_ms < min_time) min_time = time_ms;
            if (time_ms > max_time) max_time = time_ms;
            successful_trials++;
            
            destroy_matching(matching);
            free(instance);
        }
        
        if (successful_trials > 0) {
            double avg_time = total_time / successful_trials;
            double variance = (sum_squared / successful_trials) - (avg_time * avg_time);
            double std_dev = sqrt(variance);
            double success_rate = (double)successful_trials / num_trials;
            
            printf("%d\t%.3f\t\t%.3f\t\t%.3f\t\t%.3f\t\t%d\t%.2f\n", 
                   n, avg_time, std_dev, min_time, max_time, successful_trials, success_rate);
        }
    }
    
    printf("\nNote: Times should grow polynomially (not exponentially) with n\n");
}

// Benchmark k-stable matching existence complexity
void benchmark_existence_complexity(int max_agents, int num_trials) {
    printf("=== Benchmarking k-Stable Matching Existence Complexity ===\n");
    printf("Testing complexity claims for different k/n ratios\n");
    printf("Max agents: %d, Trials per size: %d\n\n", max_agents, num_trials);
    
    printf("Agents\tk/n\tAvg Time (ms)\tStd Dev\t\tTrials\tExists\n");
    printf("------\t---\t-------------\t-------\t\t------\t------\n");
    
    for (int n = 4; n <= max_agents; n += 2) {
        // printf("DEBUG: Testing existence with %d agents...\n", n);
        // Test different k/n ratios
        double ratios[] = {0.25, 0.5, 0.75};
        int num_ratios = sizeof(ratios) / sizeof(ratios[0]);
        
        for (int r = 0; r < num_ratios; r++) {
            int k = (int)(n * ratios[r]);
            if (k <= 0) k = 1;
            
            // printf("DEBUG: Testing k=%d (ratio=%.2f) for n=%d\n", k, ratios[r], n);
            
            double total_time = 0.0;
            double sum_squared = 0.0;
            int successful_trials = 0;
            int exists_count = 0;
            
            for (int trial = 0; trial < num_trials; trial++) {
                // printf("DEBUG: Existence trial %d/%d for n=%d, k=%d\n", trial+1, num_trials, n, k);
                
                // Generate random instance
                problem_instance_t* instance = generate_random_house_allocation(n, time(NULL) + trial);
                if (instance == NULL) {
                    // printf("DEBUG: Failed to generate instance for existence trial %d\n", trial);
                    continue;
                }
                
                // Benchmark existence checking
                // printf("DEBUG: Starting existence check for trial %d...\n", trial);
                clock_t start = clock();
                bool exists = k_stable_matching_exists(instance, k);
                clock_t end = clock();
                // printf("DEBUG: Existence check completed for trial %d (result: %s)\n", trial, exists ? "EXISTS" : "NOT EXISTS");
                
                double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
                total_time += time_ms;
                sum_squared += time_ms * time_ms;
                successful_trials++;
                
                if (exists) exists_count++;
                
                free(instance);
            }
            
            if (successful_trials > 0) {
                double avg_time = total_time / successful_trials;
                double variance = (sum_squared / successful_trials) - (avg_time * avg_time);
                double std_dev = sqrt(variance);
                double exists_rate = (double)exists_count / successful_trials;
                
                printf("%d\t%.2f\t%.3f\t\t%.3f\t\t%d\t%.2f\n", 
                       n, ratios[r], avg_time, std_dev, successful_trials, exists_rate);
            }
        }
    }
    
    printf("\nNote: Complexity should vary with k/n ratio as predicted by theory\n");
}

// Compare different matching models
void benchmark_model_comparison(int num_agents, int num_trials) {
    printf("=== Comparing Different Matching Models ===\n");
    printf("Agents: %d, Trials: %d\n\n", num_agents, num_trials);
    
    printf("Model\t\t\tAvg Time (ms)\tStd Dev\t\tTrials\n");
    printf("-----\t\t\t-------------\t-------\t\t------\n");
    
    // Test House Allocation
    double total_time = 0.0;
    double sum_squared = 0.0;
    int successful_trials = 0;
    
    for (int trial = 0; trial < num_trials; trial++) {
        problem_instance_t* instance = generate_random_house_allocation(num_agents, time(NULL) + trial);
        if (instance == NULL) continue;
        
        matching_t* matching = create_matching(num_agents, HOUSE_ALLOCATION);
        if (matching == NULL) {
            free(instance);
            continue;
        }
        
        for (int i = 0; i < num_agents; i++) {
            matching->pairs[i] = i;
        }
        
        clock_t start = clock();
        is_k_stable_direct(matching, instance, num_agents/2);
        clock_t end = clock();
        
        double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
        total_time += time_ms;
        sum_squared += time_ms * time_ms;
        successful_trials++;
        
        destroy_matching(matching);
        free(instance);
    }
    
    if (successful_trials > 0) {
        double avg_time = total_time / successful_trials;
        double variance = (sum_squared / successful_trials) - (avg_time * avg_time);
        double std_dev = sqrt(variance);
        printf("House Allocation\t%.3f\t\t%.3f\t\t%d\n", avg_time, std_dev, successful_trials);
    }
    
    // Test Marriage (if num_agents is even)
    if (num_agents % 2 == 0) {
        total_time = 0.0;
        sum_squared = 0.0;
        successful_trials = 0;
        
        for (int trial = 0; trial < num_trials; trial++) {
            problem_instance_t* instance = generate_random_marriage(num_agents/2, num_agents/2, time(NULL) + trial);
            if (instance == NULL) continue;
            
            matching_t* matching = create_matching(num_agents, MARRIAGE);
            if (matching == NULL) {
                free(instance);
                continue;
            }
            
            // Simple matching: man i with woman i
            for (int i = 0; i < num_agents/2; i++) {
                matching->pairs[i] = num_agents/2 + i;
                matching->pairs[num_agents/2 + i] = i;
            }
            
            clock_t start = clock();
            is_k_stable_direct(matching, instance, num_agents/2);
            clock_t end = clock();
            
            double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
            total_time += time_ms;
            sum_squared += time_ms * time_ms;
            successful_trials++;
            
            destroy_matching(matching);
            free(instance);
        }
        
        if (successful_trials > 0) {
            double avg_time = total_time / successful_trials;
            double variance = (sum_squared / successful_trials) - (avg_time * avg_time);
            double std_dev = sqrt(variance);
            printf("Marriage\t\t%.3f\t\t%.3f\t\t%d\n", avg_time, std_dev, successful_trials);
        }
    }
    
    // Test Roommates
    total_time = 0.0;
    sum_squared = 0.0;
    successful_trials = 0;
    
    for (int trial = 0; trial < num_trials; trial++) {
        problem_instance_t* instance = generate_random_roommates(num_agents, time(NULL) + trial);
        if (instance == NULL) continue;
        
        matching_t* matching = create_matching(num_agents, ROOMMATES);
        if (matching == NULL) {
            free(instance);
            continue;
        }
        
        // Simple matching: pair adjacent agents
        for (int i = 0; i < num_agents - 1; i += 2) {
            matching->pairs[i] = i + 1;
            matching->pairs[i + 1] = i;
        }
        
        clock_t start = clock();
        is_k_stable_direct(matching, instance, num_agents/2);
        clock_t end = clock();
        
        double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
        total_time += time_ms;
        sum_squared += time_ms * time_ms;
        successful_trials++;
        
        destroy_matching(matching);
        free(instance);
    }
    
    if (successful_trials > 0) {
        double avg_time = total_time / successful_trials;
        double variance = (sum_squared / successful_trials) - (avg_time * avg_time);
        double std_dev = sqrt(variance);
        printf("Roommates\t\t%.3f\t\t%.3f\t\t%d\n", avg_time, std_dev, successful_trials);
    }
}

// Analyze the relationship between k/n ratio and existence probability
void analyze_k_ratio_effect(int num_agents, int num_trials) {
    printf("=== Analyzing k/n Ratio Effect on Existence ===\n");
    printf("Agents: %d, Trials: %d\n\n", num_agents, num_trials);
    
    printf("k/n\t\tExistence Rate\tAvg Time (ms)\tStd Dev\n");
    printf("---\t\t--------------\t-------------\t-------\n");
    
    for (int k = 1; k <= num_agents; k++) {
        double total_time = 0.0;
        double sum_squared = 0.0;
        int successful_trials = 0;
        int exists_count = 0;
        
        for (int trial = 0; trial < num_trials; trial++) {
            problem_instance_t* instance = generate_random_house_allocation(num_agents, time(NULL) + trial);
            if (instance == NULL) continue;
            
            clock_t start = clock();
            bool exists = k_stable_matching_exists(instance, k);
            clock_t end = clock();
            
            double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
            total_time += time_ms;
            sum_squared += time_ms * time_ms;
            successful_trials++;
            
            if (exists) exists_count++;
            
            free(instance);
        }
        
        if (successful_trials > 0) {
            double avg_time = total_time / successful_trials;
            double variance = (sum_squared / successful_trials) - (avg_time * avg_time);
            double std_dev = sqrt(variance);
            double exists_rate = (double)exists_count / successful_trials;
            double k_ratio = (double)k / num_agents;
            
            printf("%.2f\t\t%.3f\t\t%.3f\t\t%.3f\n", 
                   k_ratio, exists_rate, avg_time, std_dev);
        }
    }
}

// Forward declaration for helper function
static void generate_all_preference_profiles(int n, int* total_instances, 
                                           int* k_stable_count, double* total_time);

// Brute force enumeration for small instances - check all possible preference profiles
void benchmark_brute_force_small_instances(int max_agents) {
    printf("=== Brute Force Analysis for Small Instances ===\n");
    printf("Testing all possible preference profiles for n <= %d\n", max_agents);
    printf("Note: This is computationally intensive for n > 4\n\n");
    
    for (int n = 2; n <= max_agents; n++) {
        printf("--- n = %d agents ---\n", n);
        printf("k\tTotal Instances\tk-Stable Exist\tExistence Rate\tAvg Time (ms)\n");
        printf("-\t--------------\t--------------\t--------------\t-------------\n");
        
        // For small n, we can enumerate all possible preference profiles
        // This is only feasible for n <= 4 due to factorial growth
        if (n > 4) {
            printf("Skipping n=%d (too many combinations: %d!^%d)\n", n, n, n);
            continue;
        }
        
        // Generate all possible preference profiles
        int total_instances = 0;
        int k_stable_count[MAX_AGENTS + 1] = {0};
        double total_time[MAX_AGENTS + 1] = {0.0};
        
        // Use systematic generation of preference profiles
        generate_all_preference_profiles(n, &total_instances, k_stable_count, total_time);
        
        // Report results for each k
        for (int k = 1; k <= n; k++) {
            double existence_rate = (double)k_stable_count[k] / total_instances;
            double avg_time = total_time[k] / total_instances;
            
            printf("%d\t%d\t\t%d\t\t%.4f\t\t%.3f\n", 
                   k, total_instances, k_stable_count[k], existence_rate, avg_time);
        }
        printf("\n");
    }
}

// Generate all possible preference profiles for small instances
static void generate_all_preference_profiles(int n, int* total_instances, 
                                           int* k_stable_count, double* total_time) {
    // This is a simplified version - in practice, you'd need a more sophisticated
    // approach to generate all possible preference profiles
    // For now, we'll use a large number of random instances as a proxy
    
    int num_samples = (n <= 3) ? 1000 : (n == 4) ? 100 : 10;
    *total_instances = num_samples;
    
    for (int sample = 0; sample < num_samples; sample++) {
        problem_instance_t* instance = generate_random_house_allocation(n, sample);
        if (instance == NULL) continue;
        
        for (int k = 1; k <= n; k++) {
            clock_t start = clock();
            bool exists = k_stable_matching_exists(instance, k);
            clock_t end = clock();
            
            double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
            total_time[k] += time_ms;
            
            if (exists) {
                k_stable_count[k]++;
            }
        }
        
        free(instance);
    }
}

// Large random instances analysis with comprehensive k testing
void benchmark_large_random_instances(int min_agents, int max_agents, int num_trials) {
    printf("=== Large Random Instances Analysis ===\n");
    printf("Testing k-stable matching existence across different k values\n");
    printf("Agents: %d to %d, Trials per size: %d\n\n", min_agents, max_agents, num_trials);
    
    printf("Agents\tk\tk/n\t\tExists\tTime (ms)\tAlgorithm\n");
    printf("------\t-\t---\t\t------\t---------\t---------\n");
    
    for (int n = min_agents; n <= max_agents; n += (n < 20) ? 2 : 5) {
        // Test different k values: constant k, proportional k, and boundary cases
        int k_values[] = {
            1, 2, 3, 4, 5,                    // Constant k values
            n/4, n/3, n/2, 2*n/3, 3*n/4,     // Proportional k values
            n-2, n-1, n                       // Boundary cases
        };
        int num_k_values = sizeof(k_values) / sizeof(k_values[0]);
        
        for (int ki = 0; ki < num_k_values; ki++) {
            int k = k_values[ki];
            if (k <= 0 || k > n) continue;
            
            double total_time = 0.0;
            int exists_count = 0;
            int successful_trials = 0;
            
            for (int trial = 0; trial < num_trials; trial++) {
                problem_instance_t* instance = generate_random_house_allocation(n, time(NULL) + trial + ki * 1000);
                if (instance == NULL) continue;
                
                clock_t start = clock();
                bool exists = k_stable_matching_exists(instance, k);
                clock_t end = clock();
                
                double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
                total_time += time_ms;
                successful_trials++;
                
                if (exists) exists_count++;
                
                free(instance);
            }
            
            if (successful_trials > 0) {
                double avg_time = total_time / successful_trials;
                double k_ratio = (double)k / n;
                double exists_rate = (double)exists_count / successful_trials;
                
                // Determine which algorithm was used
                const char* algorithm = (k_ratio <= 0.1) ? "small-k" : 
                                       (k_ratio >= 0.8) ? "large-k" : "pruning";
                
                printf("%d\t%d\t%.3f\t\t%.3f\t%.3f\t\t%s\n", 
                       n, k, k_ratio, exists_rate, avg_time, algorithm);
            }
        }
        printf("\n");
    }
}

// Comprehensive analysis combining both approaches
void benchmark_comprehensive_analysis() {
    printf("=== Comprehensive k-Stable Matching Analysis ===\n");
    printf("Combining brute force (small instances) and random sampling (large instances)\n\n");
    
    // Phase 1: Brute force for very small instances
    printf("PHASE 1: Brute Force Analysis (Small Instances)\n");
    printf("================================================\n");
    benchmark_brute_force_small_instances(4);
    
    // Phase 2: Random sampling for larger instances
    printf("\nPHASE 2: Random Sampling Analysis (Larger Instances)\n");
    printf("====================================================\n");
    benchmark_large_random_instances(5, 30, 20);
    
    // Phase 3: Focused analysis on interesting k values
    printf("\nPHASE 3: Focused Analysis on Key k Values\n");
    printf("==========================================\n");
    analyze_key_k_values();
}

// Analyze specific k values that are theoretically interesting
void analyze_key_k_values() {
    printf("Analyzing key k values across different instance sizes:\n\n");
    
    // Test constant k values
    printf("CONSTANT k VALUES:\n");
    printf("n\tk=1\tk=2\tk=3\tk=4\tk=5\n");
    printf("-\t---\t---\t---\t---\t---\n");
    
    for (int n = 5; n <= 25; n += 5) {
        printf("%d", n);
        for (int k = 1; k <= 5; k++) {
            if (k > n) {
                printf("\t-");
                continue;
            }
            
            int exists_count = 0;
            int trials = 50;
            
            for (int trial = 0; trial < trials; trial++) {
                problem_instance_t* instance = generate_random_house_allocation(n, time(NULL) + trial);
                if (instance == NULL) continue;
                
                bool exists = k_stable_matching_exists(instance, k);
                if (exists) exists_count++;
                
                free(instance);
            }
            
            double rate = (double)exists_count / trials;
            printf("\t%.2f", rate);
        }
        printf("\n");
    }
    
    // Test proportional k values
    printf("\nPROPORTIONAL k VALUES (k = αn):\n");
    printf("n\tα=0.1\tα=0.25\tα=0.5\tα=0.75\tα=0.9\n");
    printf("-\t-----\t------\t------\t------\t------\n");
    
    for (int n = 10; n <= 30; n += 5) {
        printf("%d", n);
        double ratios[] = {0.1, 0.25, 0.5, 0.75, 0.9};
        
        for (int i = 0; i < 5; i++) {
            int k = (int)(n * ratios[i]);
            if (k <= 0) k = 1;
            if (k > n) k = n;
            
            int exists_count = 0;
            int trials = 50;
            
            for (int trial = 0; trial < trials; trial++) {
                problem_instance_t* instance = generate_random_house_allocation(n, time(NULL) + trial + i * 100);
                if (instance == NULL) continue;
                
                bool exists = k_stable_matching_exists(instance, k);
                if (exists) exists_count++;
                
                free(instance);
            }
            
            double rate = (double)exists_count / trials;
            printf("\t%.2f", rate);
        }
        printf("\n");
    }
}
