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
    
    printf("Agents\tAvg Time (ms)\tStd Dev\t\tTrials\n");
    printf("------\t-------------\t-------\t\t------\n");
    
    for (int n = 5; n <= max_agents; n += 5) {
        double total_time = 0.0;
        double sum_squared = 0.0;
        int successful_trials = 0;
        
        for (int trial = 0; trial < num_trials; trial++) {
            // Generate random instance
            problem_instance_t* instance = generate_random_house_allocation(n, time(NULL) + trial);
            if (instance == NULL) continue;
            
            // Create a random matching
            matching_t* matching = create_matching(n, HOUSE_ALLOCATION);
            if (matching == NULL) {
                free(instance);
                continue;
            }
            
            // Create a simple matching (assign agent i to house i)
            for (int i = 0; i < n; i++) {
                matching->pairs[i] = i;
            }
            
            // Benchmark verification
            clock_t start = clock();
            is_k_stable_direct(matching, instance, n/2);  // k = n/2
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
            
            printf("%d\t%.3f\t\t%.3f\t\t%d\n", n, avg_time, std_dev, successful_trials);
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
        // Test different k/n ratios
        double ratios[] = {0.25, 0.5, 0.75};
        int num_ratios = sizeof(ratios) / sizeof(ratios[0]);
        
        for (int r = 0; r < num_ratios; r++) {
            int k = (int)(n * ratios[r]);
            if (k <= 0) k = 1;
            
            double total_time = 0.0;
            double sum_squared = 0.0;
            int successful_trials = 0;
            int exists_count = 0;
            
            for (int trial = 0; trial < num_trials; trial++) {
                // Generate random instance
                problem_instance_t* instance = generate_random_house_allocation(n, time(NULL) + trial);
                if (instance == NULL) continue;
                
                // Benchmark existence checking
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
