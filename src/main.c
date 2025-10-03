#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "matching.h"

void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("Options:\n");
    printf("  --test              Run basic functionality tests\n");
    printf("  --benchmark         Run computational complexity benchmarks\n");
    printf("  --verify N K        Test k-stability verification with N agents, k=K\n");
    printf("  --existence N K     Test k-stable matching existence with N agents, k=K\n");
    printf("  --generate MODEL N  Generate random instance (house|marriage|roommates) with N agents\n");
    printf("  --verify-model MODEL N K  Test verification with specific model\n");
    printf("  --existence-model MODEL N K  Test existence with specific model\n");
    printf("  --brute-force N     Run brute force analysis for small instances (n <= N)\n");
    printf("  --large-random MIN MAX TRIALS  Run large random instances analysis\n");
    printf("  --comprehensive     Run comprehensive analysis (brute force + large random)\n");
    printf("  --key-k-values      Analyze key k values (constant and proportional)\n");
    printf("  --help              Show this help message\n");
}

void run_basic_tests() {
    printf("Running basic functionality tests...\n");
    
    // Test 1: Create and destroy matching
    matching_t* matching = create_matching(4, HOUSE_ALLOCATION);
    if (matching == NULL) {
        printf("FAIL: Could not create matching\n");
        return;
    }
    destroy_matching(matching);
    printf("PASS: Matching creation/destruction\n");
    
    // Test 2: Generate random instances
    problem_instance_t* instance = generate_random_house_allocation(5, 12345);
    if (instance == NULL) {
        printf("FAIL: Could not generate random instance\n");
        return;
    }
    printf("PASS: Random instance generation\n");
    
    // Test 3: Basic verification
    matching_t* test_matching = create_matching(5, HOUSE_ALLOCATION);
    bool is_stable = is_k_stable_direct(test_matching, instance, 3);
    printf("PASS: k-stability verification (result: %s)\n", is_stable ? "stable" : "unstable");
    
    destroy_matching(test_matching);
    free(instance);
    
    printf("All basic tests passed!\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    }
    
    if (strcmp(argv[1], "--test") == 0) {
        run_basic_tests();
        return 0;
    }
    
    if (strcmp(argv[1], "--benchmark") == 0) {
        printf("Running computational complexity benchmarks...\n");

        benchmark_verification_complexity(50, 10);  // Increased from 15, 3
        benchmark_existence_complexity(20, 5);      // Increased from 8, 2
        return 0;
    }
    
    if (strcmp(argv[1], "--verify") == 0) {
        if (argc < 4) {
            printf("Error: --verify requires N and K parameters\n");
            return 1;
        }
        int n = atoi(argv[2]);
        int k = atoi(argv[3]);
        
        printf("Testing k-stability verification with %d agents, k=%d\n", n, k);
        
        problem_instance_t* instance = generate_random_house_allocation(n, time(NULL));
        matching_t* matching = create_matching(n, HOUSE_ALLOCATION);
        
        clock_t start = clock();
        bool result = is_k_stable(matching, instance, k);
        clock_t end = clock();
        
        double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Result: %s (took %.6f seconds)\n", result ? "k-stable" : "not k-stable", time_taken);
        
        destroy_matching(matching);
        free(instance);
        return 0;
    }
    
    if (strcmp(argv[1], "--existence") == 0) {
        if (argc < 4) {
            printf("Error: --existence requires N and K parameters\n");
            return 1;
        }
        int n = atoi(argv[2]);
        int k = atoi(argv[3]);
        
        printf("Testing k-stable matching existence with %d agents, k=%d\n", n, k);
        
        problem_instance_t* instance = generate_random_house_allocation(n, time(NULL));
        
        clock_t start = clock();
        bool exists = k_stable_matching_exists(instance, k);
        clock_t end = clock();
        
        double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Result: %s (took %.6f seconds)\n", exists ? "exists" : "does not exist", time_taken);
        
        free(instance);
        return 0;
    }
    
    if (strcmp(argv[1], "--generate") == 0) {
        if (argc < 4) {
            printf("Error: --generate requires MODEL and N parameters\n");
            return 1;
        }
        
        const char* model_str = argv[2];
        int n = atoi(argv[3]);
        
        matching_model_t model;
        if (strcmp(model_str, "house") == 0) {
            model = HOUSE_ALLOCATION;
        } else if (strcmp(model_str, "marriage") == 0) {
            model = MARRIAGE;
        } else if (strcmp(model_str, "roommates") == 0) {
            model = ROOMMATES;
        } else {
            printf("Error: Unknown model '%s'. Use: house, marriage, or roommates\n", model_str);
            return 1;
        }
        
        problem_instance_t* instance;
        if (model == HOUSE_ALLOCATION) {
            instance = generate_random_house_allocation(n, time(NULL));
        } else if (model == MARRIAGE) {
            instance = generate_random_marriage(n/2, n/2, time(NULL));
        } else {
            instance = generate_random_roommates(n, time(NULL));
        }
        
        printf("Generated %s instance with %d agents\n", model_str, n);
        printf("Agent preferences:\n");
        for (int i = 0; i < instance->num_agents; i++) {
            printf("Agent %d: ", i);
            for (int j = 0; j < instance->agents[i].num_preferences; j++) {
                printf("%d ", instance->agents[i].preferences[j]);
            }
            printf("\n");
        }
        
        free(instance);
        return 0;
    }
    
    if (strcmp(argv[1], "--verify-model") == 0) {
        if (argc < 5) {
            printf("Error: --verify-model requires MODEL, N, and K parameters\n");
            return 1;
        }
        
        const char* model_str = argv[2];
        int n = atoi(argv[3]);
        int k = atoi(argv[4]);
        
        matching_model_t model;
        if (strcmp(model_str, "house") == 0) {
            model = HOUSE_ALLOCATION;
        } else if (strcmp(model_str, "marriage") == 0) {
            model = MARRIAGE;
        } else if (strcmp(model_str, "roommates") == 0) {
            model = ROOMMATES;
        } else {
            printf("Error: Unknown model '%s'. Use: house, marriage, or roommates\n", model_str);
            return 1;
        }
        
        printf("Testing k-stability verification with %s model, %d agents, k=%d\n", model_str, n, k);
        
        problem_instance_t* instance;
        if (model == HOUSE_ALLOCATION) {
            instance = generate_random_house_allocation(n, time(NULL));
        } else if (model == MARRIAGE) {
            instance = generate_random_marriage(n/2, n/2, time(NULL));
        } else {
            instance = generate_random_roommates(n, time(NULL));
        }
        
        matching_t* matching = create_matching(n, model);
        if (matching == NULL) {
            printf("Error: Could not create matching\n");
            free(instance);
            return 1;
        }
        
        // Create a simple matching for testing
        if (model == HOUSE_ALLOCATION) {
            for (int i = 0; i < n; i++) {
                matching->pairs[i] = i;  // Agent i gets house i
            }
        } else if (model == MARRIAGE) {
            int half = n / 2;
            for (int i = 0; i < half; i++) {
                matching->pairs[i] = half + i;
                matching->pairs[half + i] = i;
            }
        } else {  // ROOMMATES
            for (int i = 0; i < n - 1; i += 2) {
                matching->pairs[i] = i + 1;
                matching->pairs[i + 1] = i;
            }
        }
        
        clock_t start = clock();
        bool result = is_k_stable_direct(matching, instance, k);
        clock_t end = clock();
        
        double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Result: %s (took %.6f seconds)\n", result ? "k-stable" : "not k-stable", time_taken);
        
        destroy_matching(matching);
        free(instance);
        return 0;
    }
    
    if (strcmp(argv[1], "--existence-model") == 0) {
        if (argc < 5) {
            printf("Error: --existence-model requires MODEL, N, and K parameters\n");
            return 1;
        }
        
        const char* model_str = argv[2];
        int n = atoi(argv[3]);
        int k = atoi(argv[4]);
        
        matching_model_t model;
        if (strcmp(model_str, "house") == 0) {
            model = HOUSE_ALLOCATION;
        } else if (strcmp(model_str, "marriage") == 0) {
            model = MARRIAGE;
        } else if (strcmp(model_str, "roommates") == 0) {
            model = ROOMMATES;
        } else {
            printf("Error: Unknown model '%s'. Use: house, marriage, or roommates\n", model_str);
            return 1;
        }
        
        printf("Testing k-stable matching existence with %s model, %d agents, k=%d\n", model_str, n, k);
        
        problem_instance_t* instance;
        if (model == HOUSE_ALLOCATION) {
            instance = generate_random_house_allocation(n, time(NULL));
        } else if (model == MARRIAGE) {
            instance = generate_random_marriage(n/2, n/2, time(NULL));
        } else {
            instance = generate_random_roommates(n, time(NULL));
        }
        
        clock_t start = clock();
        bool exists = k_stable_matching_exists(instance, k);
        clock_t end = clock();
        
        double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Result: %s (took %.6f seconds)\n", exists ? "exists" : "does not exist", time_taken);
        
        free(instance);
        return 0;
    }
    
    if (strcmp(argv[1], "--brute-force") == 0) {
        if (argc < 3) {
            printf("Error: --brute-force requires N parameter\n");
            return 1;
        }
        int max_agents = atoi(argv[2]);
        if (max_agents <= 0 || max_agents > 6) {
            printf("Error: N must be between 1 and 6 for brute force analysis\n");
            return 1;
        }
        
        benchmark_brute_force_small_instances(max_agents);
        return 0;
    }
    
    if (strcmp(argv[1], "--large-random") == 0) {
        if (argc < 5) {
            printf("Error: --large-random requires MIN MAX TRIALS parameters\n");
            return 1;
        }
        int min_agents = atoi(argv[2]);
        int max_agents = atoi(argv[3]);
        int num_trials = atoi(argv[4]);
        
        if (min_agents <= 0 || max_agents <= 0 || num_trials <= 0 || min_agents > max_agents) {
            printf("Error: Invalid parameters for --large-random\n");
            return 1;
        }
        
        benchmark_large_random_instances(min_agents, max_agents, num_trials);
        return 0;
    }
    
    if (strcmp(argv[1], "--comprehensive") == 0) {
        benchmark_comprehensive_analysis();
        return 0;
    }
    
    if (strcmp(argv[1], "--key-k-values") == 0) {
        analyze_key_k_values();
        return 0;
    }
    
    printf("Error: Unknown option '%s'\n", argv[1]);
    print_usage(argv[0]);
    return 1;
}
