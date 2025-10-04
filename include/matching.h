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
    ROOMMATES,
    HOUSE_ALLOCATION_PARTIAL  // k-hai with partial preferences
} matching_model_t;

// Agent structure
typedef struct {
    int id;
    int preferences[MAX_AGENTS];  // Preference list (0 = most preferred)
    int num_preferences;
    bool has_indifferences;       // For k-hai: whether agent has ties in preferences
    int indifference_groups[MAX_AGENTS]; // For k-hai: group objects with same preference
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
        struct {
            int num_houses;
            int num_acceptable_objects[MAX_AGENTS]; // For k-hai: number of acceptable objects per agent
        } house_partial_data;
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

// k-hai (partial preferences) generators
problem_instance_t* generate_k_hai_instance(int num_agents, int num_objects, uint32_t seed);
problem_instance_t* generate_k_hai_with_indifferences(int num_agents, int num_objects, uint32_t seed);
bool is_object_acceptable_to_agent(const agent_t* agent, int object_id, int num_objects);
bool agent_indifferent_between(const agent_t* agent, int obj1, int obj2);

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

// k-hai benchmarking functions
void benchmark_k_hai_comparison(int num_agents, int num_objects, int num_trials);
void benchmark_partial_vs_complete_preferences(int num_agents, int num_trials);
void analyze_k_hai_existence_patterns(int num_agents, int num_objects, int num_trials);

#endif // MATCHING_H
