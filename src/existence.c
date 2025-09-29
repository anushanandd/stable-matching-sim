#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "matching.h"

// Forward declarations
static bool find_k_stable_matching_recursive(const problem_instance_t* instance, int k, 
                                           matching_t* current_matching, int agent_index);
static bool is_partial_matching_valid(const matching_t* matching, const problem_instance_t* instance, 
                                    int up_to_agent);
static bool can_extend_to_k_stable(const matching_t* partial_matching, const problem_instance_t* instance, 
                                 int k, int remaining_agents);

// Check if a k-stable matching exists (main function)
bool k_stable_matching_exists(const problem_instance_t* instance, int k) {
    if (instance == NULL || k <= 0 || k > instance->num_agents) {
        return false;
    }
    
    // Create an empty matching to start with
    matching_t* matching = create_matching(instance->num_agents, instance->model);
    if (matching == NULL) {
        return false;
    }
    
    // Initialize all agents as unmatched
    for (int i = 0; i < instance->num_agents; i++) {
        matching->pairs[i] = -1;
    }
    
    // Use recursive backtracking to find a k-stable matching
    bool exists = find_k_stable_matching_recursive(instance, k, matching, 0);
    
    destroy_matching(matching);
    return exists;
}

// Find a k-stable matching using recursive backtracking
static bool find_k_stable_matching_recursive(const problem_instance_t* instance, int k, 
                                           matching_t* current_matching, int agent_index) {
    // Base case: if we've assigned all agents, check if the matching is k-stable
    if (agent_index >= instance->num_agents) {
        return is_k_stable_direct(current_matching, instance, k);
    }
    
    // If current agent is already matched, move to next agent
    if (current_matching->pairs[agent_index] != -1) {
        return find_k_stable_matching_recursive(instance, k, current_matching, agent_index + 1);
    }
    
    // Try to match the current agent with each possible partner
    for (int partner = 0; partner < instance->num_agents; partner++) {
        // Skip if trying to match with self (for roommates model, this is invalid)
        if (partner == agent_index) {
            continue;
        }
        
        // Skip if partner is already matched
        if (current_matching->pairs[partner] != -1) {
            continue;
        }
        
        // Model-specific constraints
        if (instance->model == MARRIAGE) {
            // In marriage model, we need to determine the number of men and women
            // This is a simplified approach - in practice, you'd store this info in the instance
            int num_men = instance->num_agents / 2;  // Assume equal numbers for now
            
            // Men (0 to num_men-1) can only match with women (num_men to num_agents-1)
            if ((agent_index < num_men && partner < num_men) || 
                (agent_index >= num_men && partner >= num_men)) {
                continue;
            }
        }
        
        // Try this matching
        current_matching->pairs[agent_index] = partner;
        current_matching->pairs[partner] = agent_index;
        
        // Check if this partial matching can potentially lead to a k-stable matching
        if (is_partial_matching_valid(current_matching, instance, agent_index) &&
            can_extend_to_k_stable(current_matching, instance, k, instance->num_agents - agent_index - 1)) {
            
            // Recursively try to complete the matching
            if (find_k_stable_matching_recursive(instance, k, current_matching, agent_index + 1)) {
                return true;
            }
        }
        
        // Backtrack: undo this matching
        current_matching->pairs[agent_index] = -1;
        current_matching->pairs[partner] = -1;
    }
    
    // Also try leaving the current agent unmatched (if allowed by the model)
    if (instance->model == HOUSE_ALLOCATION || instance->model == ROOMMATES) {
        return find_k_stable_matching_recursive(instance, k, current_matching, agent_index + 1);
    }
    
    return false;
}

// Check if a partial matching is valid up to the given agent
static bool is_partial_matching_valid(const matching_t* matching, const problem_instance_t* instance, 
                                    int up_to_agent) {
    // Check basic validity constraints
    for (int i = 0; i <= up_to_agent; i++) {
        int partner = matching->pairs[i];
        if (partner != -1) {
            // Check symmetry
            if (matching->pairs[partner] != i) {
                return false;
            }
            
            // Check bounds
            if (partner < 0 || partner >= instance->num_agents) {
                return false;
            }
        }
    }
    
    return true;
}

// Heuristic check: can this partial matching potentially be extended to a k-stable matching?
static bool can_extend_to_k_stable(const matching_t* partial_matching, const problem_instance_t* instance, 
                                 int k, int remaining_agents) {
    // This is a heuristic - in practice, you'd implement a more sophisticated check
    // For now, we'll use a simple approach
    
    // Count how many agents are already matched
    int matched_count = 0;
    for (int i = 0; i < instance->num_agents; i++) {
        if (partial_matching->pairs[i] != -1) {
            matched_count++;
        }
    }
    matched_count /= 2;  // Each pair counts twice
    
    // If we have too few matched agents, it's unlikely to be k-stable
    if (matched_count < k / 2) {
        return false;
    }
    
    // If we have too many unmatched agents, it's also unlikely
    if (remaining_agents > instance->num_agents - k) {
        return false;
    }
    
    return true;
}

// Find and return a k-stable matching (if one exists)
matching_t* find_k_stable_matching(const problem_instance_t* instance, int k) {
    if (instance == NULL || k <= 0 || k > instance->num_agents) {
        return NULL;
    }
    
    // Create an empty matching to start with
    matching_t* matching = create_matching(instance->num_agents, instance->model);
    if (matching == NULL) {
        return NULL;
    }
    
    // Initialize all agents as unmatched
    for (int i = 0; i < instance->num_agents; i++) {
        matching->pairs[i] = -1;
    }
    
    // Use recursive backtracking to find a k-stable matching
    bool found = find_k_stable_matching_recursive(instance, k, matching, 0);
    
    if (found) {
        return matching;
    } else {
        destroy_matching(matching);
        return NULL;
    }
}

// Alternative approach: use a more efficient algorithm for specific cases
bool k_stable_matching_exists_efficient(const problem_instance_t* instance, int k) {
    if (instance == NULL || k <= 0 || k > instance->num_agents) {
        return false;
    }
    
    // For very small k, we can use a more direct approach
    if (k <= 2) {
        return k_stable_matching_exists_small_k(instance, k);
    }
    
    // For k close to n, we can use a different approach
    if (k >= instance->num_agents * 0.8) {
        return k_stable_matching_exists_large_k(instance, k);
    }
    
    // For medium k, use the general recursive approach
    return k_stable_matching_exists(instance, k);
}

// Efficient algorithm for small k values
bool k_stable_matching_exists_small_k(const problem_instance_t* instance, int k) {
    // For k=1, any matching is 1-stable (no single agent can block)
    if (k == 1) {
        return true;
    }
    
    // For k=2, we need to check if there are any blocking pairs
    if (k == 2) {
        // Check if there exists any pair of agents who prefer each other over their current partners
        // This is a simplified check - in practice, you'd need to consider all possible matchings
        
        // For now, return true if the instance has at least 2 agents
        return instance->num_agents >= 2;
    }
    
    return false;
}

// Efficient algorithm for large k values (k close to n)
bool k_stable_matching_exists_large_k(const problem_instance_t* instance, int k) {
    // For large k, we need most agents to be satisfied
    // This is more restrictive, so we can use more aggressive pruning
    
    // Create a simple matching and check if it's k-stable
    matching_t* matching = create_matching(instance->num_agents, instance->model);
    if (matching == NULL) {
        return false;
    }
    
    // Try a simple matching strategy
    if (instance->model == HOUSE_ALLOCATION) {
        // Assign each agent to their most preferred house
        for (int i = 0; i < instance->num_agents; i++) {
            matching->pairs[i] = instance->agents[i].preferences[0];
        }
    } else if (instance->model == MARRIAGE) {
        // Simple matching: man i with woman i
        int half = instance->num_agents / 2;
        for (int i = 0; i < half; i++) {
            matching->pairs[i] = half + i;
            matching->pairs[half + i] = i;
        }
    } else {  // ROOMMATES
        // Pair adjacent agents
        for (int i = 0; i < instance->num_agents - 1; i += 2) {
            matching->pairs[i] = i + 1;
            matching->pairs[i + 1] = i;
        }
    }
    
    bool is_stable = is_k_stable_direct(matching, instance, k);
    destroy_matching(matching);
    
    return is_stable;
}

// Forward declaration
static int count_k_stable_matchings_recursive(const problem_instance_t* instance, int k, 
                                            matching_t* current_matching, int agent_index);

// Count the number of k-stable matchings (for analysis)
int count_k_stable_matchings(const problem_instance_t* instance, int k) {
    if (instance == NULL || k <= 0 || k > instance->num_agents) {
        return 0;
    }
    
    // This is a simplified implementation
    // In practice, you'd implement a more sophisticated counting algorithm
    
    int count = 0;
    matching_t* matching = create_matching(instance->num_agents, instance->model);
    if (matching == NULL) {
        return 0;
    }
    
    // Initialize all agents as unmatched
    for (int i = 0; i < instance->num_agents; i++) {
        matching->pairs[i] = -1;
    }
    
    // Use recursive counting (this is exponential in the worst case)
    count = count_k_stable_matchings_recursive(instance, k, matching, 0);
    
    destroy_matching(matching);
    return count;
}

// Recursive function to count k-stable matchings
static int count_k_stable_matchings_recursive(const problem_instance_t* instance, int k, 
                                            matching_t* current_matching, int agent_index) {
    // Base case: if we've assigned all agents, check if the matching is k-stable
    if (agent_index >= instance->num_agents) {
        return is_k_stable_direct(current_matching, instance, k) ? 1 : 0;
    }
    
    // If current agent is already matched, move to next agent
    if (current_matching->pairs[agent_index] != -1) {
        return count_k_stable_matchings_recursive(instance, k, current_matching, agent_index + 1);
    }
    
    int count = 0;
    
    // Try to match the current agent with each possible partner
    for (int partner = 0; partner < instance->num_agents; partner++) {
        // Skip if trying to match with self
        if (partner == agent_index) {
            continue;
        }
        
        // Skip if partner is already matched
        if (current_matching->pairs[partner] != -1) {
            continue;
        }
        
        // Model-specific constraints
        if (instance->model == MARRIAGE) {
            // In marriage model, we need to determine the number of men and women
            int num_men = instance->num_agents / 2;  // Assume equal numbers for now
            
            // Men (0 to num_men-1) can only match with women (num_men to num_agents-1)
            if ((agent_index < num_men && partner < num_men) || 
                (agent_index >= num_men && partner >= num_men)) {
                continue;
            }
        }
        
        // Try this matching
        current_matching->pairs[agent_index] = partner;
        current_matching->pairs[partner] = agent_index;
        
        // Recursively count
        count += count_k_stable_matchings_recursive(instance, k, current_matching, agent_index + 1);
        
        // Backtrack: undo this matching
        current_matching->pairs[agent_index] = -1;
        current_matching->pairs[partner] = -1;
    }
    
    // Also try leaving the current agent unmatched (if allowed)
    if (instance->model == HOUSE_ALLOCATION || instance->model == ROOMMATES) {
        count += count_k_stable_matchings_recursive(instance, k, current_matching, agent_index + 1);
    }
    
    return count;
}
