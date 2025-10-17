#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/matching.h"

// Create a new matching
matching_t* create_matching(int num_agents, matching_model_t model) {
    if (num_agents <= 0 || num_agents > MAX_AGENTS) {
        return NULL;
    }
    
    matching_t* matching = malloc(sizeof(matching_t));
    if (matching == NULL) {
        return NULL;
    }
    
    matching->num_agents = num_agents;
    matching->model = model;
    
    // Initialize all agents as unmatched
    for (int i = 0; i < num_agents; i++) {
        matching->pairs[i] = -1;
    }
    
    return matching;
}

// Destroy a matching
void destroy_matching(matching_t* matching) {
    if (matching != NULL) {
        free(matching);
    }
}

// Print a matching in a readable format
void print_matching(const matching_t* matching) {
    if (matching == NULL) {
        printf("NULL matching\n");
        return;
    }
    
    printf("Matching (model: %d, agents: %d):\n", matching->model, matching->num_agents);
    for (int i = 0; i < matching->num_agents; i++) {
        if (matching->pairs[i] != -1) {
            printf("  Agent %d <-> Agent %d\n", i, matching->pairs[i]);
        } else {
            printf("  Agent %d <-> UNMATCHED\n", i);
        }
    }
}

// Get the rank of a target agent in an agent's preference list
// Returns -1 if target is not in preferences
int get_agent_rank(const agent_t* agent, int target_id) {
    for (int i = 0; i < agent->num_preferences; i++) {
        if (agent->preferences[i] == target_id) {
            return i;  // 0 = most preferred
        }
    }
    return -1;  // Not found
}

// Check if agent prefers a over b
bool agent_prefers(const agent_t* agent, int a, int b) {
    // Special case: if b is -1 (unmatched), agent always prefers a if a is valid
    if (b == -1) {
        return get_agent_rank(agent, a) != -1;
    }
    
    // Special case: if a is -1 (unmatched), agent never prefers it
    if (a == -1) {
        return false;
    }
    
    int rank_a = get_agent_rank(agent, a);
    int rank_b = get_agent_rank(agent, b);
    
    // If either is not in preferences, return false
    if (rank_a == -1 || rank_b == -1) {
        return false;
    }
    
    // Lower rank = more preferred
    return rank_a < rank_b;
}

// Count how many agents are better off in alternative matching compared to current
int count_improved_agents(const matching_t* current, const matching_t* alternative, 
                         const problem_instance_t* instance) {
    if (current == NULL || alternative == NULL || instance == NULL) {
        return 0;
    }
    
    int improved_count = 0;
    
    for (int i = 0; i < instance->num_agents; i++) {
        int current_partner = current->pairs[i];
        int alternative_partner = alternative->pairs[i];
        
        // Skip if agent is unmatched in both
        if (current_partner == -1 && alternative_partner == -1) {
            continue;
        }
        
        // Agent is better off if:
        // 1. Was unmatched, now matched
        // 2. Was matched, now matched to someone they prefer
        bool is_better = false;
        
        if (current_partner == -1 && alternative_partner != -1) {
            // Was unmatched, now matched
            is_better = true;
        } else if (current_partner != -1 && alternative_partner != -1) {
            // Both matched, check preference
            is_better = agent_prefers(&instance->agents[i], alternative_partner, current_partner);
        }
        
        if (is_better) {
            improved_count++;
        }
    }
    
    return improved_count;
}

// Check if a matching is valid for the given model
bool is_valid_matching(const matching_t* matching, const problem_instance_t* instance) {
    if (matching == NULL || instance == NULL) {
        return false;
    }
    
    if (matching->num_agents != instance->num_agents) {
        return false;
    }
    
    // Check that pairs are symmetric
    for (int i = 0; i < matching->num_agents; i++) {
        int partner = matching->pairs[i];
        if (partner != -1) {
            if (partner < 0 || partner >= matching->num_agents) {
                return false;  // Invalid partner ID
            }
            if (matching->pairs[partner] != i) {
                return false;  // Not symmetric
            }
        }
    }
    
    // Model-specific validation
    switch (matching->model) {
        case HOUSE_ALLOCATION:
            {
            // In house allocation, each house can only be assigned to one agent
            // and each agent can get at most one house
            // Note: pairs[i] represents the house assigned to agent i (-1 if no house)
            bool house_assigned[MAX_AGENTS] = {false};
            
            for (int i = 0; i < matching->num_agents; i++) {
                int house = matching->pairs[i];
                if (house != -1) {
                    // Check that house ID is valid
                    if (house < 0 || house >= matching->num_agents) {
                        return false;
                    }
                    // Check that this house is not assigned to multiple agents
                    if (house_assigned[house]) {
                        return false;  // House assigned to multiple agents
                    }
                    house_assigned[house] = true;
                }
            }
            }
            break;
            
        case MARRIAGE:
            // In marriage model, men can only be matched with women and vice versa
            // Use the metadata to determine gender boundaries
            for (int i = 0; i < matching->num_agents; i++) {
                int partner = matching->pairs[i];
                if (partner != -1) {
                    // Check that men are matched with women and vice versa
                    int num_men = instance->model_data.marriage_data.num_men;
                    bool i_is_man = (i < num_men);
                    bool partner_is_man = (partner < num_men);
                    
                    if (i_is_man == partner_is_man) {
                        return false;  // Same gender matching
                    }
                }
            }
            break;
            
        case ROOMMATES:
            // In roommates model, any agent can be matched with any other agent
            // No additional constraints beyond symmetry
            break;
            
        case HOUSE_ALLOCATION_PARTIAL:
            // Similar to house allocation but with partial preferences
            // Each house can only be assigned to one agent
            {
            bool house_assigned[MAX_AGENTS] = {false};
            
            for (int i = 0; i < matching->num_agents; i++) {
                int house = matching->pairs[i];
                if (house != -1) {
                    // Check that house ID is valid
                    if (house < 0 || house >= instance->model_data.house_partial_data.num_houses) {
                        return false;
                    }
                    // Check that this house is not assigned to multiple agents
                    if (house_assigned[house]) {
                        return false;  // House assigned to multiple agents
                    }
                    house_assigned[house] = true;
                }
            }
            }
            break;
    }
    
    return true;
}

// Create a copy of a matching
matching_t* copy_matching(const matching_t* original) {
    if (original == NULL) {
        return NULL;
    }
    
    matching_t* copy = create_matching(original->num_agents, original->model);
    if (copy == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < original->num_agents; i++) {
        copy->pairs[i] = original->pairs[i];
    }
    
    return copy;
}
