#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "include/matching.h"

// Standalone program for brute force house allocation analysis
int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <n> <k>\n", argv[0]);
        printf("  n: number of agents/objects (1-8)\n");
        printf("  k: stability parameter (1-n)\n");
        printf("\nExample: %s 3 2\n", argv[0]);
        return 1;
    }
    
    int n = atoi(argv[1]);
    int k = atoi(argv[2]);
    
    if (n <= 0 || n > 8) {
        printf("Error: n must be between 1 and 8 for brute force analysis\n");
        return 1;
    }
    
    if (k <= 0 || k > n) {
        printf("Error: k must be between 1 and n\n");
        return 1;
    }
    
    printf("Brute Force House Allocation Analysis\n");
    printf("=====================================\n");
    printf("n = %d agents/objects, k = %d\n\n", n, k);
    
    // Run the analysis
    analyze_all_house_allocations(n, k);
    
    return 0;
}
