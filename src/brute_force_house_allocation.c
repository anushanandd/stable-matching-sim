#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../include/matching.h"

// Structure to hold matching analysis results
typedef struct {
    matching_t* matching;
    int agents_preferring_others;
    bool is_k_stable;
} matching_analysis_t;

// Forward declarations
static void generate_all_matchings_recursive(int n, int* current_matching, int agent_index, 
                                           bool* used_objects, matching_analysis_t* results, 
                                           int* result_count, const problem_instance_t* instance, int k);
static int count_agents_preferring_others(const matching_t* matching, const problem_instance_t* instance);
static bool is_matching_k_stable(const matching_t* matching, const problem_instance_t* instance, int k);
static void print_matching_analysis(const matching_analysis_t* analysis, int matching_index);
static long long factorial(int n);

// Main function to analyze all possible matchings for house allocation
void analyze_all_house_allocations(int n, int k) {
    if (n <= 0 || n > 8) {  // Limit to prevent excessive computation
        printf("Error: n must be between 1 and 8 for brute force analysis\n");
        return;
    }
    
    if (k <= 0 || k > n) {
        printf("Error: k must be between 1 and n\n");
        return;
    }
    
    printf("=== Brute Force House Allocation Analysis ===\n");
    printf("n = %d agents/objects, k = %d\n", n, k);
    
    // Generate a random house allocation instance
    problem_instance_t* instance = generate_random_house_allocation(n, 12345);
    if (instance == NULL) {
        printf("Error: Could not generate problem instance\n");
        return;
    }
    
    printf("\nProblem Instance:\n");
    print_problem_instance(instance);
    
    // Calculate total number of possible matchings
    long long total_matchings = factorial(n);
    printf("\nTotal possible matchings: %lld\n", total_matchings);
    
    // Allocate space for results
    matching_analysis_t* results = malloc(total_matchings * sizeof(matching_analysis_t));
    if (results == NULL) {
        printf("Error: Could not allocate memory for results\n");
        free(instance);
        return;
    }
    
    // Initialize tracking variables
    int* current_matching = malloc(n * sizeof(int));
    bool* used_objects = malloc(n * sizeof(bool));
    int result_count = 0;
    
    if (current_matching == NULL || used_objects == NULL) {
        printf("Error: Could not allocate memory for tracking variables\n");
        free(results);
        free(current_matching);
        free(used_objects);
        free(instance);
        return;
    }
    
    // Initialize used_objects array
    for (int i = 0; i < n; i++) {
        used_objects[i] = false;
    }
    
    printf("\nGenerating and analyzing all matchings...\n");
    
    // Generate all possible matchings and analyze them
    generate_all_matchings_recursive(n, current_matching, 0, used_objects, results, 
                                   &result_count, instance, k);
    
    printf("Analysis complete! Generated %d matchings.\n\n", result_count);
    
    // Print summary statistics
    int k_stable_count = 0;
    int total_preferring_others = 0;
    int min_preferring = n;
    int max_preferring = 0;
    
    for (int i = 0; i < result_count; i++) {
        if (results[i].is_k_stable) {
            k_stable_count++;
        }
        total_preferring_others += results[i].agents_preferring_others;
        if (results[i].agents_preferring_others < min_preferring) {
            min_preferring = results[i].agents_preferring_others;
        }
        if (results[i].agents_preferring_others > max_preferring) {
            max_preferring = results[i].agents_preferring_others;
        }
    }
    
    printf("=== SUMMARY STATISTICS ===\n");
    printf("Total matchings: %d\n", result_count);
    printf("k-stable matchings: %d (%.2f%%)\n", k_stable_count, 
           (double)k_stable_count / result_count * 100);
    printf("Average agents preferring others: %.2f\n", 
           (double)total_preferring_others / result_count);
    printf("Min agents preferring others: %d\n", min_preferring);
    printf("Max agents preferring others: %d\n", max_preferring);
    
    // Print detailed results for small instances
    if (n <= 4) {
        printf("\n=== DETAILED RESULTS ===\n");
        for (int i = 0; i < result_count; i++) {
            print_matching_analysis(&results[i], i);
        }
    }
    
    // Clean up
    for (int i = 0; i < result_count; i++) {
        destroy_matching(results[i].matching);
    }
    free(results);
    free(current_matching);
    free(used_objects);
    free(instance);
}

// Recursively generate all possible matchings
static void generate_all_matchings_recursive(int n, int* current_matching, int agent_index, 
                                           bool* used_objects, matching_analysis_t* results, 
                                           int* result_count, const problem_instance_t* instance, int k) {
    if (agent_index == n) {
        // We have a complete matching
        matching_t* matching = create_matching(n, HOUSE_ALLOCATION);
        if (matching == NULL) {
            return;
        }
        
        // Set the matching
        for (int i = 0; i < n; i++) {
            matching->pairs[i] = current_matching[i];
        }
        
        // Analyze this matching
        results[*result_count].matching = matching;
        results[*result_count].agents_preferring_others = 
            count_agents_preferring_others(matching, instance);
        results[*result_count].is_k_stable = 
            is_matching_k_stable(matching, instance, k);
        
        (*result_count)++;
        return;
    }
    
    // Try each unused object for the current agent
    for (int obj = 0; obj < n; obj++) {
        if (!used_objects[obj]) {
            current_matching[agent_index] = obj;
            used_objects[obj] = true;
            
            generate_all_matchings_recursive(n, current_matching, agent_index + 1, 
                                           used_objects, results, result_count, instance, k);
            
            used_objects[obj] = false;
        }
    }
}

// Count how many agents prefer other matchings over the current one
static int count_agents_preferring_others(const matching_t* matching, const problem_instance_t* instance) {
    int count = 0;
    int n = instance->num_agents;
    
    for (int agent = 0; agent < n; agent++) {
        int current_object = matching->pairs[agent];
        int current_rank = get_agent_rank(&instance->agents[agent], current_object);
        
        // Check if there's any other object this agent prefers more
        bool prefers_other = false;
        for (int pref_rank = 0; pref_rank < current_rank; pref_rank++) {
            int preferred_object = instance->agents[agent].preferences[pref_rank];
            // If the preferred object is assigned to someone else, this agent would prefer a different matching
            if (preferred_object != current_object) {
                prefers_other = true;
                break;
            }
        }
        
        if (prefers_other) {
            count++;
        }
    }
    
    return count;
}

// Check if a matching is k-stable
static bool is_matching_k_stable(const matching_t* matching, const problem_instance_t* instance, int k) {
    return is_k_stable_direct(matching, instance, k);
}

// Print analysis of a single matching
static void print_matching_analysis(const matching_analysis_t* analysis, int matching_index) {
    printf("Matching %d: ", matching_index);
    
    // Print the matching
    printf("[");
    for (int i = 0; i < analysis->matching->num_agents; i++) {
        printf("%d->%d", i, analysis->matching->pairs[i]);
        if (i < analysis->matching->num_agents - 1) {
            printf(", ");
        }
    }
    printf("] ");
    
    printf("Agents preferring others: %d, ", analysis->agents_preferring_others);
    printf("k-stable: %s\n", analysis->is_k_stable ? "YES" : "NO");
}

// Calculate factorial
static long long factorial(int n) {
    if (n <= 1) return 1;
    long long result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

// Function to run analysis with different parameters
void run_brute_force_analysis() {
    printf("Running brute force house allocation analysis...\n\n");
    
    // Test with different values of n and k
    int test_cases[][2] = {
        {2, 1}, {2, 2},
        {3, 1}, {3, 2}, {3, 3},
        {4, 1}, {4, 2}, {4, 3}, {4, 4}
    };
    
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (int i = 0; i < num_test_cases; i++) {
        int n = test_cases[i][0];
        int k = test_cases[i][1];
        
        printf("========================================\n");
        analyze_all_house_allocations(n, k);
        printf("\n");
    }
}
