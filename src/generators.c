#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/matching.h"

// Improved random number generator using xorshift32
static uint32_t rng_state = 1;

static uint32_t xorshift32() {
    // Xorshift32 algorithm - much better quality than LCG
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

static void rng_seed(uint32_t seed) {
    // Ensure seed is never 0 (would break xorshift)
    rng_state = (seed == 0) ? 1 : seed;
    
    // Warm up the generator
    for (int i = 0; i < 10; i++) {
        xorshift32();
    }
}

// Wrapper for compatibility
static uint32_t lcg_rand() {
    return xorshift32();
}

static void lcg_seed(uint32_t seed) {
    rng_seed(seed);
}

// Fisher-Yates shuffle algorithm
static void shuffle_array(int* array, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = lcg_rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

// Generate random house allocation instance
problem_instance_t* generate_random_house_allocation(int num_agents, uint32_t seed) {
    if (num_agents <= 0 || num_agents > MAX_AGENTS) {
        return NULL;
    }
    
    lcg_seed(seed);
    
    problem_instance_t* instance = malloc(sizeof(problem_instance_t));
    if (instance == NULL) {
        return NULL;
    }
    
    instance->num_agents = num_agents;
    instance->model = HOUSE_ALLOCATION;
    instance->model_data.house_data.num_houses = num_agents;
    
    // Initialize agents
    for (int i = 0; i < num_agents; i++) {
        instance->agents[i].id = i;
        instance->agents[i].num_preferences = num_agents;
        
        // Create preference list (houses 0 to num_agents-1)
        for (int j = 0; j < num_agents; j++) {
            instance->agents[i].preferences[j] = j;
        }
        
        // Shuffle preferences for each agent
        shuffle_array(instance->agents[i].preferences, num_agents);
    }
    
    return instance;
}

// Generate random marriage instance
problem_instance_t* generate_random_marriage(int num_men, int num_women, uint32_t seed) {
    if (num_men <= 0 || num_women <= 0 || (num_men + num_women) > MAX_AGENTS) {
        return NULL;
    }
    
    lcg_seed(seed);
    
    problem_instance_t* instance = malloc(sizeof(problem_instance_t));
    if (instance == NULL) {
        return NULL;
    }
    
    instance->num_agents = num_men + num_women;
    instance->model = MARRIAGE;
    instance->model_data.marriage_data.num_men = num_men;
    instance->model_data.marriage_data.num_women = num_women;
    
    // Initialize men (agents 0 to num_men-1)
    for (int i = 0; i < num_men; i++) {
        instance->agents[i].id = i;
        instance->agents[i].num_preferences = num_women;
        
        // Create preference list (women num_men to num_men+num_women-1)
        for (int j = 0; j < num_women; j++) {
            instance->agents[i].preferences[j] = num_men + j;
        }
        
        // Shuffle preferences
        shuffle_array(instance->agents[i].preferences, num_women);
    }
    
    // Initialize women (agents num_men to num_men+num_women-1)
    for (int i = 0; i < num_women; i++) {
        int woman_id = num_men + i;
        instance->agents[woman_id].id = woman_id;
        instance->agents[woman_id].num_preferences = num_men;
        
        // Create preference list (men 0 to num_men-1)
        for (int j = 0; j < num_men; j++) {
            instance->agents[woman_id].preferences[j] = j;
        }
        
        // Shuffle preferences
        shuffle_array(instance->agents[woman_id].preferences, num_men);
    }
    
    return instance;
}

// Generate random roommates instance
problem_instance_t* generate_random_roommates(int num_agents, uint32_t seed) {
    if (num_agents <= 0 || num_agents > MAX_AGENTS) {
        return NULL;
    }
    
    lcg_seed(seed);
    
    problem_instance_t* instance = malloc(sizeof(problem_instance_t));
    if (instance == NULL) {
        return NULL;
    }
    
    instance->num_agents = num_agents;
    instance->model = ROOMMATES;
    // Note: For roommates, odd numbers mean one agent will remain unmatched
    
    // Initialize agents
    for (int i = 0; i < num_agents; i++) {
        instance->agents[i].id = i;
        instance->agents[i].num_preferences = num_agents - 1;  // Can't prefer themselves
        
        // Create preference list (all other agents)
        int pref_count = 0;
        for (int j = 0; j < num_agents; j++) {
            if (j != i) {  // Don't include self in preferences
                instance->agents[i].preferences[pref_count] = j;
                pref_count++;
            }
        }
        
        // Shuffle preferences
        shuffle_array(instance->agents[i].preferences, num_agents - 1);
    }
    
    return instance;
}

// Generate a specific test case for debugging
problem_instance_t* generate_test_case_1() {
    // Simple 3-agent house allocation case
    problem_instance_t* instance = malloc(sizeof(problem_instance_t));
    if (instance == NULL) {
        return NULL;
    }
    
    instance->num_agents = 3;
    instance->model = HOUSE_ALLOCATION;
    instance->model_data.house_data.num_houses = 3;
    
    // Agent 0: prefers house 1 > 2 > 0
    instance->agents[0].id = 0;
    instance->agents[0].num_preferences = 3;
    instance->agents[0].preferences[0] = 1;
    instance->agents[0].preferences[1] = 2;
    instance->agents[0].preferences[2] = 0;
    
    // Agent 1: prefers house 2 > 0 > 1
    instance->agents[1].id = 1;
    instance->agents[1].num_preferences = 3;
    instance->agents[1].preferences[0] = 2;
    instance->agents[1].preferences[1] = 0;
    instance->agents[1].preferences[2] = 1;
    
    // Agent 2: prefers house 0 > 1 > 2
    instance->agents[2].id = 2;
    instance->agents[2].num_preferences = 3;
    instance->agents[2].preferences[0] = 0;
    instance->agents[2].preferences[1] = 1;
    instance->agents[2].preferences[2] = 2;
    
    return instance;
}

// Generate a test case where k-stable matching exists
problem_instance_t* generate_k_stable_exists_case(int num_agents, int k) {
    if (num_agents <= 0 || num_agents > MAX_AGENTS || k <= 0 || k > num_agents) {
        return NULL;
    }
    
    problem_instance_t* instance = malloc(sizeof(problem_instance_t));
    if (instance == NULL) {
        return NULL;
    }
    
    instance->num_agents = num_agents;
    instance->model = HOUSE_ALLOCATION;
    instance->model_data.house_data.num_houses = num_agents;
    
    // Create a case where agents have very similar preferences
    // This makes it more likely that a k-stable matching exists
    for (int i = 0; i < num_agents; i++) {
        instance->agents[i].id = i;
        instance->agents[i].num_preferences = num_agents;
        
        // Each agent prefers houses in a similar order
        for (int j = 0; j < num_agents; j++) {
            instance->agents[i].preferences[j] = (i + j) % num_agents;
        }
    }
    
    return instance;
}

// Generate a test case where k-stable matching is unlikely to exist
problem_instance_t* generate_k_stable_unlikely_case(int num_agents, int k) {
    if (num_agents <= 0 || num_agents > MAX_AGENTS || k <= 0 || k > num_agents) {
        return NULL;
    }
    
    problem_instance_t* instance = malloc(sizeof(problem_instance_t));
    if (instance == NULL) {
        return NULL;
    }
    
    instance->num_agents = num_agents;
    instance->model = HOUSE_ALLOCATION;
    instance->model_data.house_data.num_houses = num_agents;
    
    // Create a case where agents have very different preferences
    // This makes it less likely that a k-stable matching exists
    for (int i = 0; i < num_agents; i++) {
        instance->agents[i].id = i;
        instance->agents[i].num_preferences = num_agents;
        
        // Each agent has completely different preferences
        for (int j = 0; j < num_agents; j++) {
            instance->agents[i].preferences[j] = (num_agents - 1 - j + i) % num_agents;
        }
    }
    
    return instance;
}

// Print a problem instance
void print_problem_instance(const problem_instance_t* instance) {
    if (instance == NULL) {
        printf("NULL instance\n");
        return;
    }
    
    const char* model_names[] = {"House Allocation", "Marriage", "Roommates"};
    printf("Problem Instance (Model: %s, Agents: %d):\n", 
           model_names[instance->model], instance->num_agents);
    
    for (int i = 0; i < instance->num_agents; i++) {
        printf("  Agent %d preferences: ", i);
        for (int j = 0; j < instance->agents[i].num_preferences; j++) {
            printf("%d ", instance->agents[i].preferences[j]);
        }
        printf("\n");
    }
}

// Generate k-hai instance with partial preferences
problem_instance_t* generate_k_hai_instance(int num_agents, int num_objects, uint32_t seed) {
    if (num_agents <= 0 || num_objects <= 0 || num_agents > MAX_AGENTS || num_objects > MAX_AGENTS) {
        return NULL;
    }
    
    lcg_seed(seed);
    
    problem_instance_t* instance = malloc(sizeof(problem_instance_t));
    if (instance == NULL) {
        return NULL;
    }
    
    instance->num_agents = num_agents;
    instance->model = HOUSE_ALLOCATION_PARTIAL;
    instance->model_data.house_partial_data.num_houses = num_objects;
    
    // Initialize agents with partial preferences
    for (int i = 0; i < num_agents; i++) {
        instance->agents[i].id = i;
        instance->agents[i].has_indifferences = false;
        
        // Determine how many objects this agent finds acceptable (at least 1, at most num_objects)
        int num_acceptable = 1 + (lcg_rand() % num_objects);
        instance->agents[i].num_preferences = num_acceptable;
        instance->model_data.house_partial_data.num_acceptable_objects[i] = num_acceptable;
        
        // Create list of all objects
        int all_objects[MAX_AGENTS];
        for (int j = 0; j < num_objects; j++) {
            all_objects[j] = j;
        }
        
        // Shuffle and take first num_acceptable objects
        shuffle_array(all_objects, num_objects);
        
        // Set preferences (only over acceptable objects)
        for (int j = 0; j < num_acceptable; j++) {
            instance->agents[i].preferences[j] = all_objects[j];
        }
        
        // Initialize indifference groups (no indifferences by default)
        for (int j = 0; j < num_acceptable; j++) {
            instance->agents[i].indifference_groups[j] = j; // Each object in its own group
        }
    }
    
    return instance;
}

// Generate k-hai instance with indifferences
problem_instance_t* generate_k_hai_with_indifferences(int num_agents, int num_objects, uint32_t seed) {
    if (num_agents <= 0 || num_objects <= 0 || num_agents > MAX_AGENTS || num_objects > MAX_AGENTS) {
        return NULL;
    }
    
    lcg_seed(seed);
    
    problem_instance_t* instance = malloc(sizeof(problem_instance_t));
    if (instance == NULL) {
        return NULL;
    }
    
    instance->num_agents = num_agents;
    instance->model = HOUSE_ALLOCATION_PARTIAL;
    instance->model_data.house_partial_data.num_houses = num_objects;
    
    // Initialize agents with partial preferences and indifferences
    for (int i = 0; i < num_agents; i++) {
        instance->agents[i].id = i;
        
        // Determine how many objects this agent finds acceptable
        int num_acceptable = 1 + (lcg_rand() % num_objects);
        instance->agents[i].num_preferences = num_acceptable;
        instance->model_data.house_partial_data.num_acceptable_objects[i] = num_acceptable;
        
        // Create list of all objects
        int all_objects[MAX_AGENTS];
        for (int j = 0; j < num_objects; j++) {
            all_objects[j] = j;
        }
        
        // Shuffle and take first num_acceptable objects
        shuffle_array(all_objects, num_objects);
        
        // Set preferences
        for (int j = 0; j < num_acceptable; j++) {
            instance->agents[i].preferences[j] = all_objects[j];
        }
        
        // Create indifferences (ties) in preferences
        instance->agents[i].has_indifferences = (lcg_rand() % 3 == 0); // 1/3 chance of having indifferences
        
        if (instance->agents[i].has_indifferences && num_acceptable >= 2) {
            // Create some indifference groups
            int num_groups = 1 + (lcg_rand() % (num_acceptable / 2 + 1));
            int group_id = 0;
            
            for (int j = 0; j < num_acceptable; j++) {
                instance->agents[i].indifference_groups[j] = group_id;
                // Move to next group with some probability
                if (j < num_acceptable - 1 && lcg_rand() % 3 == 0) {
                    group_id++;
                }
            }
        } else {
            // No indifferences
            for (int j = 0; j < num_acceptable; j++) {
                instance->agents[i].indifference_groups[j] = j;
            }
        }
    }
    
    return instance;
}

// Check if an object is acceptable to an agent (for k-hai)
bool is_object_acceptable_to_agent(const agent_t* agent, int object_id, int num_objects) {
    if (agent == NULL || object_id < 0 || object_id >= num_objects) {
        return false;
    }
    
    // Check if object is in agent's preference list
    for (int i = 0; i < agent->num_preferences; i++) {
        if (agent->preferences[i] == object_id) {
            return true;
        }
    }
    
    return false;
}

// Check if an agent is indifferent between two objects (for k-hai)
bool agent_indifferent_between(const agent_t* agent, int obj1, int obj2) {
    if (agent == NULL || !agent->has_indifferences) {
        return false;
    }
    
    // Find the positions of obj1 and obj2 in preferences
    int pos1 = -1, pos2 = -1;
    for (int i = 0; i < agent->num_preferences; i++) {
        if (agent->preferences[i] == obj1) pos1 = i;
        if (agent->preferences[i] == obj2) pos2 = i;
    }
    
    // If either object is not in preferences, they're not indifferent
    if (pos1 == -1 || pos2 == -1) {
        return false;
    }
    
    // Check if they're in the same indifference group
    return agent->indifference_groups[pos1] == agent->indifference_groups[pos2];
}
