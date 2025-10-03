#ifndef MATCHING_H
#define MATCHING_H

#include <stdbool.h>
#include <stdint.h>

// Maximum number of agents for static allocation
#define MAX_AGENTS 1000

// Matching models
typedef enum {
    HOUSE_ALLOCATION,
    MARRIAGE,
    ROOMMATES
} matching_model_t;

// Agent structure
typedef struct {
    int id;
    int preferences[MAX_AGENTS];  // Preference list (0 = most preferred)
    int num_preferences;
} agent_t;

// Matching structure
typedef struct {
    int pairs[MAX_AGENTS];  // pairs[i] = j means agent i is matched with agent j, -1 if unmatched
    int num_agents;
    matching_model_t model;
} matching_t;

// Problem instance
typedef struct {
    agent_t agents[MAX_AGENTS];
    int num_agents;
    matching_model_t model;
    // Model-specific metadata
    union {
        struct {
            int num_men;
            int num_women;
        } marriage_data;
        struct {
            int num_houses;
        } house_data;
    } model_data;
} problem_instance_t;

// Function declarations

// Core matching functions
matching_t* create_matching(int num_agents, matching_model_t model);
void destroy_matching(matching_t* matching);
void print_matching(const matching_t* matching);

// k-stability verification (polynomial time)
bool is_k_stable(const matching_t* matching, const problem_instance_t* instance, int k);
bool is_k_stable_direct(const matching_t* matching, const problem_instance_t* instance, int k);

// k-stable matching existence checking
bool k_stable_matching_exists(const problem_instance_t* instance, int k);
matching_t* find_k_stable_matching(const problem_instance_t* instance, int k);
bool k_stable_matching_exists_efficient(const problem_instance_t* instance, int k);
bool k_stable_matching_exists_small_k(const problem_instance_t* instance, int k);
bool k_stable_matching_exists_large_k(const problem_instance_t* instance, int k);
int count_k_stable_matchings(const problem_instance_t* instance, int k);

// Utility functions
int get_agent_rank(const agent_t* agent, int target_id);
bool agent_prefers(const agent_t* agent, int a, int b);
int count_improved_agents(const matching_t* current, const matching_t* alternative, 
                         const problem_instance_t* instance);
bool is_valid_matching(const matching_t* matching, const problem_instance_t* instance);
matching_t* copy_matching(const matching_t* original);

// Test case generators
problem_instance_t* generate_random_house_allocation(int num_agents, uint32_t seed);
problem_instance_t* generate_random_marriage(int num_men, int num_women, uint32_t seed);
problem_instance_t* generate_random_roommates(int num_agents, uint32_t seed);
problem_instance_t* generate_test_case_1(void);
problem_instance_t* generate_k_stable_exists_case(int num_agents, int k);
problem_instance_t* generate_k_stable_unlikely_case(int num_agents, int k);
void print_problem_instance(const problem_instance_t* instance);

// Benchmarking
void benchmark_verification_complexity(int max_agents, int num_trials);
void benchmark_existence_complexity(int max_agents, int num_trials);
void benchmark_model_comparison(int num_agents, int num_trials);
void analyze_k_ratio_effect(int num_agents, int num_trials);

// Enhanced benchmarking functions
void benchmark_brute_force_small_instances(int max_agents);
void benchmark_large_random_instances(int min_agents, int max_agents, int num_trials);
void benchmark_comprehensive_analysis(void);
void analyze_key_k_values(void);

#endif // MATCHING_H
