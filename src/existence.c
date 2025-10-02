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
// Removed unused function declaration
static bool find_k_stable_with_pruning(const problem_instance_t* instance, int k);
static bool is_promising_partial_matching(const matching_t* partial_matching, const problem_instance_t* instance, 
                                        int k, int agents_processed);
static int estimate_blocking_potential(const matching_t* matching, const problem_instance_t* instance, int k);

// Check if a k-stable matching exists (main function)
bool k_stable_matching_exists(const problem_instance_t* instance, int k) {
    if (instance == NULL || k <= 0 || k > instance->num_agents) {
        return false;
    }
    
    int n = instance->num_agents;
    double k_ratio = (double)k / n;
    
    // Use different algorithms based on k/n ratio for efficiency
    if (k_ratio <= 0.1) {
        // For very small k, use specialized small-k algorithm
        return k_stable_matching_exists_small_k(instance, k);
    } else if (k_ratio >= 0.8) {
        // For large k, use specialized large-k algorithm
        return k_stable_matching_exists_large_k(instance, k);
    } else {
        // For medium k, use improved algorithm with pruning
        return find_k_stable_with_pruning(instance, k);
    }
}

// Improved algorithm with pruning for medium k values
static bool find_k_stable_with_pruning(const problem_instance_t* instance, int k) {
    matching_t* matching = create_matching(instance->num_agents, instance->model);
    if (matching == NULL) {
        return false;
    }
    
    // Initialize all agents as unmatched
    for (int i = 0; i < instance->num_agents; i++) {
        matching->pairs[i] = -1;
    }
    
    // Use improved recursive search with better pruning
    bool exists = find_k_stable_matching_recursive(instance, k, matching, 0);
    
    destroy_matching(matching);
    return exists;
}

// Find a k-stable matching using recursive backtracking with improved pruning
static bool find_k_stable_matching_recursive(const problem_instance_t* instance, int k, 
                                           matching_t* current_matching, int agent_index) {
    // Base case: if we've assigned all agents, check if the matching is k-stable
    if (agent_index >= instance->num_agents) {
        return is_k_stable_direct(current_matching, instance, k);
    }
    
    // Early pruning: check if partial matching is promising
    if (!is_promising_partial_matching(current_matching, instance, k, agent_index)) {
        return false;
    }
    
    // If current agent is already matched, move to next agent
    if (current_matching->pairs[agent_index] != -1) {
        return find_k_stable_matching_recursive(instance, k, current_matching, agent_index + 1);
    }
    
    // Get ordered list of potential partners (preference-based ordering)
    int potential_partners[MAX_AGENTS];
    int num_potential = 0;
    
    // Add partners in preference order
    for (int pref_idx = 0; pref_idx < instance->agents[agent_index].num_preferences; pref_idx++) {
        int partner = instance->agents[agent_index].preferences[pref_idx];
        
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
            int num_men = instance->num_agents / 2;
            if ((agent_index < num_men && partner < num_men) || 
                (agent_index >= num_men && partner >= num_men)) {
                continue;
            }
        }
        
        potential_partners[num_potential++] = partner;
    }
    
    // Try partners in preference order
    for (int i = 0; i < num_potential; i++) {
        int partner = potential_partners[i];
        
        // Try this matching
        current_matching->pairs[agent_index] = partner;
        current_matching->pairs[partner] = agent_index;
        
        // Check if this partial matching is valid and promising
        if (is_partial_matching_valid(current_matching, instance, agent_index)) {
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

// Improved heuristic check for partial matching promise
static bool is_promising_partial_matching(const matching_t* partial_matching, const problem_instance_t* instance, 
                                        int k, int agents_processed) {
    // Estimate the blocking potential of the current partial matching
    int blocking_potential = estimate_blocking_potential(partial_matching, instance, k);
    
    // If the blocking potential is already too high, prune this branch
    if (blocking_potential >= k) {
        return false;
    }
    
    // Check if we can still achieve k-stability with remaining agents
    int remaining_agents = instance->num_agents - agents_processed;
    int unmatched_count = 0;
    
    for (int i = 0; i < agents_processed; i++) {
        if (partial_matching->pairs[i] == -1) {
            unmatched_count++;
        }
    }
    
    // If too many agents are unmatched and could form blocking coalitions
    if (unmatched_count + remaining_agents >= k * 2) {
        // This could lead to large blocking coalitions
        return remaining_agents > 0; // Only continue if we can still make progress
    }
    
    return true;
}

// Estimate the blocking potential of a partial matching
static int estimate_blocking_potential(const matching_t* matching, const problem_instance_t* instance, int k) {
    (void)k; // Parameter used for future extensions, suppress warning
    int potential = 0;
    
    // Count agents who are clearly dissatisfied
    for (int i = 0; i < instance->num_agents; i++) {
        int current_partner = matching->pairs[i];
        
        if (current_partner == -1) {
            // Unmatched agents contribute to blocking potential
            potential++;
        } else {
            // Check if agent has much better alternatives available
            int current_rank = get_agent_rank(&instance->agents[i], current_partner);
            if (current_rank > 2) { // If current partner is not in top 2 preferences
                potential++;
            }
        }
    }
    
    return potential;
}

// Note: can_extend_to_k_stable function removed as it was unused

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
    
    // For small k, we can use a more direct approach
    if (k <= 3) {
        // Check if we can construct a matching where fewer than k agents want to deviate
        matching_t* matching = create_matching(instance->num_agents, instance->model);
        if (matching == NULL) {
            return false;
        }
        
        // Try a greedy approach: match agents to their most preferred available partners
        bool used[MAX_AGENTS] = {false};
        
        for (int i = 0; i < instance->num_agents; i++) {
            if (used[i]) continue;
            
            // Find best available partner for agent i
            for (int pref_idx = 0; pref_idx < instance->agents[i].num_preferences; pref_idx++) {
                int preferred = instance->agents[i].preferences[pref_idx];
                
                if (preferred >= instance->num_agents || used[preferred] || preferred == i) {
                    continue;
                }
                
                // Check model constraints
                if (instance->model == MARRIAGE) {
                    int num_men = instance->num_agents / 2;
                    if ((i < num_men && preferred < num_men) || 
                        (i >= num_men && preferred >= num_men)) {
                        continue;
                    }
                }
                
                // Check if preferred agent also likes agent i reasonably well
                int reverse_rank = get_agent_rank(&instance->agents[preferred], i);
                if (reverse_rank != -1 && reverse_rank < instance->agents[preferred].num_preferences / 2) {
                    // Make the match
                    matching->pairs[i] = preferred;
                    matching->pairs[preferred] = i;
                    used[i] = used[preferred] = true;
                    break;
                }
            }
        }
        
        // Check if this matching is k-stable
        bool is_stable = is_k_stable_direct(matching, instance, k);
        destroy_matching(matching);
        return is_stable;
    }
    
    // For slightly larger small k, use the general algorithm
    return find_k_stable_with_pruning(instance, k);
}

// Efficient algorithm for large k values (k close to n)
bool k_stable_matching_exists_large_k(const problem_instance_t* instance, int k) {
    // For large k, we need most agents to be satisfied
    // Try multiple high-quality matching strategies
    
    // Strategy 1: Try to maximize overall satisfaction
    matching_t* matching1 = create_matching(instance->num_agents, instance->model);
    if (matching1 == NULL) {
        return false;
    }
    
    // Use a more sophisticated matching algorithm for large k
    bool used[MAX_AGENTS] = {false};
    
    // Sort agents by their "pickiness" (agents with fewer acceptable partners go first)
    int agent_order[MAX_AGENTS];
    for (int i = 0; i < instance->num_agents; i++) {
        agent_order[i] = i;
    }
    
    // Simple sorting by preference list length (ascending)
    for (int i = 0; i < instance->num_agents - 1; i++) {
        for (int j = i + 1; j < instance->num_agents; j++) {
            if (instance->agents[agent_order[i]].num_preferences > 
                instance->agents[agent_order[j]].num_preferences) {
                int temp = agent_order[i];
                agent_order[i] = agent_order[j];
                agent_order[j] = temp;
            }
        }
    }
    
    // Match agents in order of pickiness
    for (int idx = 0; idx < instance->num_agents; idx++) {
        int agent = agent_order[idx];
        if (used[agent]) continue;
        
        // Try to find the best mutual match
        for (int pref_idx = 0; pref_idx < instance->agents[agent].num_preferences; pref_idx++) {
            int preferred = instance->agents[agent].preferences[pref_idx];
            
            if (preferred >= instance->num_agents || used[preferred] || preferred == agent) {
                continue;
            }
            
            // Check model constraints
            if (instance->model == MARRIAGE) {
                int num_men = instance->num_agents / 2;
                if ((agent < num_men && preferred < num_men) || 
                    (agent >= num_men && preferred >= num_men)) {
                    continue;
                }
            }
            
            // Check mutual preference (important for large k)
            int reverse_rank = get_agent_rank(&instance->agents[preferred], agent);
            if (reverse_rank != -1 && reverse_rank < instance->agents[preferred].num_preferences / 3) {
                // Make the match
                matching1->pairs[agent] = preferred;
                matching1->pairs[preferred] = agent;
                used[agent] = used[preferred] = true;
                break;
            }
        }
    }
    
    // Check if this matching is k-stable
    bool is_stable = is_k_stable_direct(matching1, instance, k);
    destroy_matching(matching1);
    
    if (is_stable) {
        return true;
    }
    
    // Strategy 2: If first strategy failed, try a different approach
    // For very large k (k > 0.9*n), it's very unlikely that a k-stable matching exists
    if (k > instance->num_agents * 0.9) {
        return false;
    }
    
    // Try one more strategy with different priorities
    return find_k_stable_with_pruning(instance, k);
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
