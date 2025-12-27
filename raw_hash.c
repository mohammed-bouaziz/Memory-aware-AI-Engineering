#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TABLE_SIZE 20000000
#define EMPTY_KEY -1

// No pointer, Contiguous data, ensured locality

typedef struct {
  int key;
  int value;
  int is_full;
} Entry;

Entry *table;

// Hash func, small changes in input = big change in output
uint32_t hash(int key) {
  uint32_t x = key;
  x ^= x >> 16;
  x *= 0x85ebca6b;
  x ^= x >> 13;
  x *= 0xc2b2ae35;
  x ^= x >> 16;
  return x;
}

void insert(int key, int value) {
  uint32_t index = hash(key) % TABLE_SIZE;
  while (table[index].is_full) {
    if (table[index].key == key) {
      table[index].value = value;
      return;
    }
    index = (index + 1) % TABLE_SIZE;
  }
  table[index].key = key;
  table[index].value = value;
  table[index].is_full = 1;
}

int get(int key) {
  uint32_t index = hash(key) % TABLE_SIZE;
  while (table[index].is_full) {
    if (table[index].key == key) {
      return table[index].value;
    }
    index = (index + 1) % TABLE_SIZE;
  }
  return -1;
}

int main() {
  printf("___ C Raw HM Benchmark ___\n");

  printf("Allocating table (%lu MB)...\n",
         (sizeof(Entry) * TABLE_SIZE) / 1024 / 1024);

  table = malloc(sizeof(Entry) * TABLE_SIZE);

  memset(table, 0, sizeof(Entry) * TABLE_SIZE);

  int N = 10000000;
  printf("Inserting %d items...\n", N);

  clock_t start = clock();

  for (int i = 0; i < N; i++) {
    insert(i, i * 2);
  }
  clock_t end = clock();

  double time_taken = (double)(end - start) / CLOCKS_PER_SEC;
  printf("Insert Time: %.4f seconds\n", time_taken);
  printf("throughput: %.2f million ops/sec\n", (N / time_taken) / 1e6);

  printf("\nReading back %d items...\n", N);
  start = clock();
  long long checksum = 0;
  for (int i = 0; i < N; i++) {
    checksum += get(i);
  }
  end = clock();
  time_taken = (double)(end - start) / CLOCKS_PER_SEC;
  printf("Lookup time: %.4f seconds\n", time_taken);
  printf("throughput: %.2f million ops/sec\n", (N / time_taken) / 1e6);
  printf("checksum: %lld\n", checksum);

  free(table);
  return 0;
}
