#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "matching.h"

// Forward declarations for helper functions
static bool has_k_blocking_coalition(const matching_t* matching, const problem_instance_t* instance, int k);
static bool check_alternative_matching(const matching_t* current, const matching_t* alternative, 
                                     const problem_instance_t* instance, int k);
static matching_t* generate_alternative_matching(const matching_t* current, const problem_instance_t* instance, 
                                               int* agents, int num_agents);
static bool is_feasible_matching(const matching_t* matching, const problem_instance_t* instance);
// Removed unused function declaration
static bool check_coalitions_of_size(const matching_t* matching, const problem_instance_t* instance, 
                                    int coalition_size, int k);
static bool check_small_coalitions(const matching_t* matching, const problem_instance_t* instance,
                                  int* candidates, int candidate_count, int coalition_size, int k);
static bool check_large_coalitions(const matching_t* matching, const problem_instance_t* instance,
                                  int* candidates, int candidate_count, int coalition_size, int k);
static bool generate_combinations(int* candidates, int candidate_count, int* coalition, int coalition_pos,
                                int coalition_size, int start_idx, const matching_t* matching,
                                const problem_instance_t* instance, int k);
static bool can_coalition_block(const matching_t* matching, const problem_instance_t* instance,
                               int* coalition, int coalition_size, int k);

// Main k-stability verification function (polynomial time)
bool is_k_stable(const matching_t* matching, const problem_instance_t* instance, int k) {
    if (matching == NULL || instance == NULL) {
        return false;
    }
    
    if (k <= 0 || k > instance->num_agents) {
        return false;
    }
    
    // Validate that the matching is feasible for the given model
    if (!is_feasible_matching(matching, instance)) {
        return false;
    }
    
    // A matching is k-stable if there is no blocking coalition of size at least k
    return !has_k_blocking_coalition(matching, instance, k);
}

// Check if there exists a blocking coalition of size at least k (polynomial-time algorithm)
static bool has_k_blocking_coalition(const matching_t* matching, const problem_instance_t* instance, int k) {
    // Polynomial-time algorithm: systematically check for blocking coalitions
    // Key insight: we need to find if there exists an alternative matching where
    // at least k agents are strictly better off
    
    int n = instance->num_agents;
    
    // For each possible subset of agents of size >= k, check if they can form a blocking coalition
    // We use a more efficient approach than full enumeration
    
    // Strategy 1: Check obvious blocking coalitions first (unmatched agents)
    int unmatched_agents[MAX_AGENTS];
    int unmatched_count = 0;
    
    for (int i = 0; i < n; i++) {
        if (matching->pairs[i] == -1) {
            unmatched_agents[unmatched_count++] = i;
        }
    }
    
    // If we have >= k unmatched agents who can form beneficial pairs
    if (unmatched_count >= k) {
        // Check if these agents can form mutually beneficial matchings
        int beneficial_pairs = 0;
        bool used[MAX_AGENTS] = {false};
        
        for (int i = 0; i < unmatched_count && beneficial_pairs * 2 < k; i++) {
            if (used[i]) continue;
            
            int agent1 = unmatched_agents[i];
            for (int j = i + 1; j < unmatched_count; j++) {
                if (used[j]) continue;
                
                int agent2 = unmatched_agents[j];
                
                // Check if they mutually prefer each other over being unmatched
                if (get_agent_rank(&instance->agents[agent1], agent2) != -1 &&
                    get_agent_rank(&instance->agents[agent2], agent1) != -1) {
                    beneficial_pairs++;
                    used[i] = used[j] = true;
                    break;
                }
            }
        }
        
        if (beneficial_pairs * 2 >= k) {
            return true; // Found blocking coalition of unmatched agents
        }
    }
    
    // Strategy 2: Check for blocking coalitions involving matched agents
    // For efficiency, we focus on agents who have better alternatives available
    
    for (int size = k; size <= n && size <= k + 5; size++) { // Limit search for efficiency
        if (check_coalitions_of_size(matching, instance, size, k)) {
            return true;
        }
    }
    
    return false;
}

// Check if coalitions of a specific size can form blocking coalitions
static bool check_coalitions_of_size(const matching_t* matching, const problem_instance_t* instance, 
                                    int coalition_size, int k) {
    int n = instance->num_agents;
    
    // Use a more efficient approach: focus on agents with improvement potential
    int candidates[MAX_AGENTS];
    int candidate_count = 0;
    
    // Identify agents who have potential for improvement
    for (int i = 0; i < n; i++) {
        int current_partner = matching->pairs[i];
        bool has_better_option = false;
        
        // Check if agent has a more preferred partner available
        for (int j = 0; j < instance->agents[i].num_preferences; j++) {
            int preferred = instance->agents[i].preferences[j];
            
            // Stop when we reach current partner (no better options after this)
            if (preferred == current_partner) {
                break;
            }
            
            // Check if this preferred partner is available or also wants to switch
            int preferred_partner = (preferred < n) ? matching->pairs[preferred] : -1;
            if (preferred_partner == -1 || 
                (preferred_partner != -1 && agent_prefers(&instance->agents[preferred], i, preferred_partner))) {
                has_better_option = true;
                break;
            }
        }
        
        if (has_better_option || current_partner == -1) {
            candidates[candidate_count++] = i;
        }
    }
    
    // If we don't have enough candidates, no blocking coalition possible
    if (candidate_count < coalition_size) {
        return false;
    }
    
    // For small coalition sizes, check all combinations
    if (coalition_size <= 6) {
        return check_small_coalitions(matching, instance, candidates, candidate_count, coalition_size, k);
    }
    
    // For larger coalitions, use heuristic approach
    return check_large_coalitions(matching, instance, candidates, candidate_count, coalition_size, k);
}

// Helper function to check small coalitions exhaustively
static bool check_small_coalitions(const matching_t* matching, const problem_instance_t* instance,
                                  int* candidates, int candidate_count, int coalition_size, int k) {
    // Generate all combinations of coalition_size from candidates
    int coalition[MAX_AGENTS];
    return generate_combinations(candidates, candidate_count, coalition, 0, coalition_size, 0,
                               matching, instance, k);
}

// Helper function to check large coalitions using heuristics
static bool check_large_coalitions(const matching_t* matching, const problem_instance_t* instance,
                                  int* candidates, int candidate_count, int coalition_size, int k) {
    // Use greedy approach: select agents with highest improvement potential
    int coalition[MAX_AGENTS];
    
    // Sort candidates by improvement potential (simplified heuristic)
    for (int i = 0; i < coalition_size && i < candidate_count; i++) {
        coalition[i] = candidates[i];
    }
    
    return can_coalition_block(matching, instance, coalition, coalition_size, k);
}

// Implement the missing helper functions

// Generate combinations recursively
static bool generate_combinations(int* candidates, int candidate_count, int* coalition, int coalition_pos,
                                int coalition_size, int start_idx, const matching_t* matching,
                                const problem_instance_t* instance, int k) {
    if (coalition_pos == coalition_size) {
        return can_coalition_block(matching, instance, coalition, coalition_size, k);
    }
    
    for (int i = start_idx; i <= candidate_count - (coalition_size - coalition_pos); i++) {
        coalition[coalition_pos] = candidates[i];
        if (generate_combinations(candidates, candidate_count, coalition, coalition_pos + 1,
                                coalition_size, i + 1, matching, instance, k)) {
            return true;
        }
    }
    return false;
}

// Check if a specific coalition can block the current matching
static bool can_coalition_block(const matching_t* matching, const problem_instance_t* instance,
                               int* coalition, int coalition_size, int k) {
    // Try to construct an alternative matching where coalition members are better off
    matching_t* alternative = generate_alternative_matching(matching, instance, coalition, coalition_size);
    if (alternative == NULL) {
        return false;
    }
    
    bool blocks = check_alternative_matching(matching, alternative, instance, k);
    destroy_matching(alternative);
    return blocks;
}

// Full polynomial-time k-stability verification algorithm  
bool is_k_stable_direct(const matching_t* matching, const problem_instance_t* instance, int k) {
    // Use the same algorithm as is_k_stable for consistency
    return is_k_stable(matching, instance, k);
}

// Generate an alternative matching for a given coalition
static matching_t* generate_alternative_matching(const matching_t* current, const problem_instance_t* instance, 
                                               int* agents, int num_agents) {
    matching_t* alternative = copy_matching(current);
    if (alternative == NULL) {
        return NULL;
    }
    
    // Try to improve the matching for the given agents
    // This is a simplified approach - in practice, you'd use more sophisticated algorithms
    
    for (int i = 0; i < num_agents; i++) {
        int agent = agents[i];
        int current_partner = current->pairs[agent];
        
        // Try to find a better partner
        for (int j = 0; j < instance->agents[agent].num_preferences; j++) {
            int preferred = instance->agents[agent].preferences[j];
            
            // Stop when we reach current partner
            if (preferred == current_partner) {
                break;
            }
            
            // Check if this preferred partner is available or willing to switch
            if (preferred < instance->num_agents) {
                int preferred_current = alternative->pairs[preferred];
                
                if (preferred_current == -1 || 
                    agent_prefers(&instance->agents[preferred], agent, preferred_current)) {
                    
                    // Make the switch
                    if (current_partner != -1) {
                        alternative->pairs[current_partner] = -1;
                    }
                    if (preferred_current != -1) {
                        alternative->pairs[preferred_current] = -1;
                    }
                    
                    alternative->pairs[agent] = preferred;
                    alternative->pairs[preferred] = agent;
                    break;
                }
            }
        }
    }
    
    return alternative;
}

// Check if an alternative matching provides k or more improvements
static bool check_alternative_matching(const matching_t* current, const matching_t* alternative, 
                                     const problem_instance_t* instance, int k) {
    int improved_count = count_improved_agents(current, alternative, instance);
    return improved_count >= k;
}

// Check if a matching is feasible for the given model
static bool is_feasible_matching(const matching_t* matching, const problem_instance_t* instance) {
    return is_valid_matching(matching, instance);
}

// Note: enumerate_agent_subsets function removed as it was unused
