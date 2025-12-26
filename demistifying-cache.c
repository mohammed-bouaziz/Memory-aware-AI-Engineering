#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define REPEAT 100000000   // Number of accesses
#define CACHE_LINE_SIZE 64 // Typical cache line size
#define INT_SIZE sizeof(int)
#define INTS_PER_CACHE_LINE (CACHE_LINE_SIZE / INT_SIZE)

// Memory latencies in Nanoseconds
#define L1_LATENCY 1.0   // ~4 cycles at 4GHz
#define L2_LATENCY 3.0   // ~12 cycles at 4GHz
#define L3_LATENCY 10.0  // ~40 cycles at 4GHz
#define RAM_LATENCY 50.0 // ~200 cycles at 4GHz

typedef struct {
  double time_ms;
  long cache_line_loads;
  double estimated_hit_rate;
} AccessStats;

AccessStats sequential_test(int *array, int size, int accesses) {
  AccessStats stats = {0};
  long sum = 0;

  clock_t start = clock();
  for (int i = 0; i < accesses; i++) {
    sum += array[i % size];
  }
  stats.time_ms = (double)(clock() - start) * 1000 / CLOCKS_PER_SEC;

  if (sum == 0)
    printf(""); // Preventing optimization
  return stats;
}

AccessStats random_test(int *array, int *indices, int size, int accesses) {
  AccessStats stats = {0};
  long sum = 0;

  clock_t start = clock();
  for (int i = 0; i < accesses; i++) {
    sum += array[indices[i % size]];
  }
  stats.time_ms = (double)(clock() - start) * 1000 / CLOCKS_PER_SEC;

  stats.cache_line_loads = accesses;

  if (size * INT_SIZE <= 32 * 1024) {         // L1 size
    stats.estimated_hit_rate = 0.95;          // Most hits in L1
  } else if (size * INT_SIZE <= 256 * 1024) { // L2 size
    stats.estimated_hit_rate = 0.80;          // some L1 misses, mostly L2 hits
  } else if (size * INT_SIZE <= 8 * 1024 * 1024) { // L3 size
    stats.estimated_hit_rate = 0.50;               // L2/L3 Hits
  } else {
    stats.estimated_hit_rate = 0.10; // Mostly RAM access
  }

  if (sum == 0)
    printf("");
  return stats;
}

void shuffle(int *array, int size) {
  for (int i = size - 1; i > 0; i--) {
    int j = rand() % (i + 1);
    int temp = array[i];
    array[i] = array[j];
    array[j] = temp;
  }
}

double calculate_effective_access_time(double hit_rate, int size) {
  double cache_time, memory_time;

  // Determine which cache level based on size
  if (size * INT_SIZE <= 32 * 1024) {
    cache_time = L1_LATENCY;
    memory_time = L2_LATENCY; // "Miss" goes to L2
  } else if (size * INT_SIZE <= 256 * 1024) {
    cache_time = L2_LATENCY;
    memory_time = L3_LATENCY; // "Miss" goes to L3
  } else if (size * INT_SIZE <= 8 * 1024 * 1024) {
    cache_time = L3_LATENCY;
    memory_time = RAM_LATENCY; // "Miss" goes to RAM
  } else {
    cache_time = L3_LATENCY; // Even with misses, some data in L3
    memory_time = RAM_LATENCY;
  }

  // Effective Access Time = hit_rate * cache_time + (1 - hit_rate) *
  // memory_time
  return hit_rate * cache_time + (1 - hit_rate) * memory_time;
}

void print_cache_analysis(const char *pattern, AccessStats stats, int size) {
  double eat = calculate_effective_access_time(stats.estimated_hit_rate, size);
  double ns_per_access = (stats.time_ms * 1000000) / REPEAT;

  printf("\n%s Access Pattern:\n", pattern);
  printf("  Measured time: %.1f ms\n", stats.time_ms);
  printf("  Nanoseconds per access: %.2f ns\n", ns_per_access);
  printf("  Estimated cache hit rate: %.1f%%\n",
         stats.estimated_hit_rate * 100);
  printf("  Theoretical EAT: %.2f ns\n", eat);
  printf("  Cache line loads: %ld\n", stats.cache_line_loads);
  printf("  Cache efficiency: %.1f%% (accesses per cache line load)\n",
         (double)REPEAT / stats.cache_line_loads * 100);
}

int main() {
  printf("=================================================================\n");
  printf("CACHE EFFECTS DEMONSTRATION WITH EFFECTIVE ACCESS TIME\n");
  printf("=================================================================\n");
  printf("Cache line size: %d bytes (%d integers)\n", CACHE_LINE_SIZE,
         INTS_PER_CACHE_LINE);
  printf(
      "Theoretical latencies: L1=%.0fns, L2=%.0fns, L3=%.0fns, RAM=%.0fns\n\n",
      L1_LATENCY, L2_LATENCY, L3_LATENCY, RAM_LATENCY);

  // Detect actual cache sizes on the system
  printf("System's cache configuration:\n");
  system("lscpu | grep cache:");
  printf("\n");

  int sizes[] = {
      1024,    // 4 KB - fits in L1
      8192,    // 32 KB - at L1 boundary
      65536,   // 256 KB - fits in L2
      262144,  // 1 MB - exceeds L2
      2097152, // 8 MB - at L3 boundary
      16777216 // 64 MB - exceeds L3
  };

  const char *size_descriptions[] = {
      "4 KB (fits in L1)", "32 KB (L1 boundary)", "256 KB (fits in L2)",
      "1 MB (exceeds L2)", "8 MB (L3 boundary)",  "64 MB (exceeds L3, RAM)"};

  printf("=================================================================\n");
  printf("PERFORMANCE COMPARISON\n");
  printf(
      "=================================================================\n\n");

  for (int s = 0; s < 6; s++) {
    int size = sizes[s];
    printf("Array size: %s\n", size_descriptions[s]);
    printf(
        "-----------------------------------------------------------------\n");

    int *array = malloc(size * sizeof(int));
    int *indices = malloc(size * sizeof(int));

    // Initialize
    for (int i = 0; i < size; i++) {
      array[i] = i;
      indices[i] = i;
    }
    shuffle(indices, size);

    // Warm up
    sequential_test(array, size, 1000);

    // Test sequential
    AccessStats seq_stats = sequential_test(array, size, REPEAT);
    print_cache_analysis("Sequential", seq_stats, size);

    // Test random
    AccessStats rand_stats = random_test(array, indices, size, REPEAT);
    print_cache_analysis("Random", rand_stats, size);

    // Calculate speedup
    double speedup = rand_stats.time_ms / seq_stats.time_ms;
    double eat_seq =
        calculate_effective_access_time(seq_stats.estimated_hit_rate, size);
    double eat_rand =
        calculate_effective_access_time(rand_stats.estimated_hit_rate, size);

    printf("\n*** COMPARISON ***\n");
    printf("  Sequential is %.1fx faster\n", speedup);
    printf("  Sequential hit rate: %.1f%% → EAT: %.2f ns\n",
           seq_stats.estimated_hit_rate * 100, eat_seq);
    printf("  Random hit rate: %.1f%% → EAT: %.2f ns\n",
           rand_stats.estimated_hit_rate * 100, eat_rand);
    printf("  EAT improvement: %.1fx better\n", eat_rand / eat_seq);

    // Show the formula in action
    printf("\n  Effective Access Time Formula:\n");
    printf("  EAT = hit_rate × cache_time + (1 - hit_rate) × memory_time\n");
    printf("  Sequential: %.3f × %.1f + %.3f × %.1f = %.2f ns\n",
           seq_stats.estimated_hit_rate,
           (size * INT_SIZE <= 32 * 1024)    ? L1_LATENCY
           : (size * INT_SIZE <= 256 * 1024) ? L2_LATENCY
                                             : L3_LATENCY,
           1 - seq_stats.estimated_hit_rate,
           (size * INT_SIZE <= 32 * 1024)    ? L2_LATENCY
           : (size * INT_SIZE <= 256 * 1024) ? L3_LATENCY
                                             : RAM_LATENCY,
           eat_seq);
    printf("  Random: %.3f × %.1f + %.3f × %.1f = %.2f ns\n",
           rand_stats.estimated_hit_rate,
           (size * INT_SIZE <= 32 * 1024)    ? L1_LATENCY
           : (size * INT_SIZE <= 256 * 1024) ? L2_LATENCY
                                             : L3_LATENCY,
           1 - rand_stats.estimated_hit_rate,
           (size * INT_SIZE <= 32 * 1024)    ? L2_LATENCY
           : (size * INT_SIZE <= 256 * 1024) ? L3_LATENCY
                                             : RAM_LATENCY,
           eat_rand);

    printf("\n================================================================="
           "\n\n");

    free(array);
    free(indices);
  }

  return 0;
}
