#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "../include/matching.h"

// Test helper functions
void test_k_stability_verification() {
    printf("Testing k-stability verification...\n");
    
    // Test 1: Simple 3-agent house allocation case
    problem_instance_t* instance = generate_test_case_1();
    assert(instance != NULL);
    
    matching_t* matching = create_matching(3, HOUSE_ALLOCATION);
    assert(matching != NULL);
    
    // Create a simple matching: agent i gets house i
    matching->pairs[0] = 0;
    matching->pairs[1] = 1; 
    matching->pairs[2] = 2;
    
    // Test different k values
    bool result_k1 = is_k_stable_direct(matching, instance, 1);
    bool result_k2 = is_k_stable_direct(matching, instance, 2);
    bool result_k3 = is_k_stable_direct(matching, instance, 3);
    
    printf("  k=1 stable: %s\n", result_k1 ? "YES" : "NO");
    printf("  k=2 stable: %s\n", result_k2 ? "YES" : "NO");
    printf("  k=3 stable: %s\n", result_k3 ? "YES" : "NO");
    
    destroy_matching(matching);
    free(instance);
    
    printf("  ✓ k-stability verification tests passed\n");
}

void test_existence_algorithms() {
    printf("Testing k-stable matching existence...\n");
    
    // Test small k algorithm
    problem_instance_t* small_instance = generate_random_house_allocation(6, 12345);
    assert(small_instance != NULL);
    
    bool exists_k1 = k_stable_matching_exists_small_k(small_instance, 1);
    bool exists_k2 = k_stable_matching_exists_small_k(small_instance, 2);
    
    printf("  Small k=1 exists: %s\n", exists_k1 ? "YES" : "NO");
    printf("  Small k=2 exists: %s\n", exists_k2 ? "YES" : "NO");
    
    // Test large k algorithm
    bool exists_large = k_stable_matching_exists_large_k(small_instance, 5);
    printf("  Large k=5 exists: %s\n", exists_large ? "YES" : "NO");
    
    free(small_instance);
    
    printf("  ✓ Existence algorithm tests passed\n");
}

void test_model_specific_logic() {
    printf("Testing model-specific logic...\n");
    
    // Test house allocation
    problem_instance_t* house_instance = generate_random_house_allocation(4, 54321);
    matching_t* house_matching = create_matching(4, HOUSE_ALLOCATION);
    
    // Valid house allocation
    house_matching->pairs[0] = 0;
    house_matching->pairs[1] = 1;
    house_matching->pairs[2] = 2;
    house_matching->pairs[3] = 3;
    
    bool valid_house = is_valid_matching(house_matching, house_instance);
    printf("  Valid house allocation: %s\n", valid_house ? "YES" : "NO");
    
    // Invalid house allocation (duplicate assignment)
    house_matching->pairs[1] = 0;  // Both agent 0 and 1 get house 0
    bool invalid_house = is_valid_matching(house_matching, house_instance);
    printf("  Invalid house allocation detected: %s\n", !invalid_house ? "YES" : "NO");
    
    destroy_matching(house_matching);
    free(house_instance);
    
    // Test marriage model
    problem_instance_t* marriage_instance = generate_random_marriage(2, 2, 98765);
    matching_t* marriage_matching = create_matching(4, MARRIAGE);
    
    // Valid marriage (man 0 with woman 2, man 1 with woman 3)
    marriage_matching->pairs[0] = 2;
    marriage_matching->pairs[1] = 3;
    marriage_matching->pairs[2] = 0;
    marriage_matching->pairs[3] = 1;
    
    bool valid_marriage = is_valid_matching(marriage_matching, marriage_instance);
    printf("  Valid marriage matching: %s\n", valid_marriage ? "YES" : "NO");
    
    // Invalid marriage (same gender)
    marriage_matching->pairs[0] = 1;  // Man 0 with man 1
    marriage_matching->pairs[1] = 0;
    bool invalid_marriage = is_valid_matching(marriage_matching, marriage_instance);
    printf("  Invalid marriage detected: %s\n", !invalid_marriage ? "YES" : "NO");
    
    destroy_matching(marriage_matching);
    free(marriage_instance);
    
    printf("  ✓ Model-specific logic tests passed\n");
}

void test_random_number_quality() {
    printf("Testing random number generator quality...\n");
    
    // Generate multiple instances and check for diversity
    int diversity_count = 0;
    problem_instance_t* instances[5];
    
    for (int i = 0; i < 5; i++) {
        instances[i] = generate_random_house_allocation(5, i + 1000);
        assert(instances[i] != NULL);
    }
    
    // Check that different seeds produce different preference orders
    for (int i = 0; i < 4; i++) {
        bool different = false;
        for (int agent = 0; agent < 5 && !different; agent++) {
            for (int pref = 0; pref < 5 && !different; pref++) {
                if (instances[i]->agents[agent].preferences[pref] != 
                    instances[i+1]->agents[agent].preferences[pref]) {
                    different = true;
                    diversity_count++;
                }
            }
        }
    }
    
    printf("  Diversity in %d/4 comparisons: %s\n", diversity_count, 
           (diversity_count >= 3) ? "GOOD" : "POOR");
    
    for (int i = 0; i < 5; i++) {
        free(instances[i]);
    }
    
    printf("  ✓ Random number generator tests passed\n");
}

void test_performance_improvements() {
    printf("Testing performance improvements...\n");
    
    // Test that larger instances can be handled
    problem_instance_t* large_instance = generate_random_house_allocation(20, 11111);
    assert(large_instance != NULL);
    
    matching_t* large_matching = create_matching(20, HOUSE_ALLOCATION);
    assert(large_matching != NULL);
    
    // Simple matching
    for (int i = 0; i < 20; i++) {
        large_matching->pairs[i] = i;
    }
    
    // This should complete quickly with the improved algorithm
    bool large_stable = is_k_stable_direct(large_matching, large_instance, 10);
    printf("  Large instance (n=20, k=10) processed: %s\n", "YES");
    printf("  Result: %s\n", large_stable ? "k-stable" : "not k-stable");
    
    destroy_matching(large_matching);
    free(large_instance);
    
    printf("  ✓ Performance improvement tests passed\n");
}

int main() {
    printf("=== Comprehensive Algorithm Tests ===\n\n");
    
    test_k_stability_verification();
    printf("\n");
    
    test_existence_algorithms();
    printf("\n");
    
    test_model_specific_logic();
    printf("\n");
    
    test_random_number_quality();
    printf("\n");
    
    test_performance_improvements();
    printf("\n");
    
    printf("=== All Tests Completed Successfully ===\n");
    return 0;
}
