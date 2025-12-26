#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 10000000 // 10 Million nodes

typedef struct Node {
  int value;
  struct Node *next;
} Node;

// This mimics a naive implementation of a fragmented heap

Node *create_scattered_list() {
  Node *head = malloc(sizeof(Node));
  Node *current = head;
  for (int i = 0; i < N; i++) {
    current->value = i;
    current->next = malloc(sizeof(Node));
    current = current->next;
  }
  current->next = NULL;
  return head;
}

// Here we malloc one giant block, called the Arena in CS
Node *create_arena_list() {
  // One single system call (conceptually)
  Node *pool = malloc((N + 1) * sizeof(Node));

  for (int i = 0; i < N; i++) {
    pool[i].value = i;
    // The 'next' pointer points to the very next slot in RAM
    pool[i].next = &pool[i + 1];
  }
  pool[N].next = NULL;
  return &pool[0];
}

long long traverse_list(Node *head) {
  long long sum = 0;
  Node *current = head;
  while (current != NULL) {
    sum += current->value;
    current = current->next;
  }
  return sum;
}

int main() {
  printf("Comparing Allocation Strategies for %d nodes...\n", N);
  printf("sizeof(Node) = %lu bytes\n\n", sizeof(Node));

  // --- TEST 1: SCATTERED ---
  printf("1. SCATTERED ALLOCATION (Standard malloc loop)\n");

  clock_t start = clock();
  Node *scatter_head = create_scattered_list();
  clock_t end = clock();
  printf("   Allocation Time: %.3f seconds\n",
         (double)(end - start) / CLOCKS_PER_SEC);

  start = clock();
  long long sum_scatter = traverse_list(scatter_head);
  end = clock();
  printf("   Traversal Time:  %.3f seconds\n",
         (double)(end - start) / CLOCKS_PER_SEC);

  // --- TEST 2: ARENA ---
  printf("\n2. ARENA ALLOCATION (Single massive malloc)\n");

  start = clock();
  Node *arena_head = create_arena_list();
  end = clock();
  printf("   Allocation Time: %.3f seconds\n",
         (double)(end - start) / CLOCKS_PER_SEC);

  start = clock();
  long long sum_arena = traverse_list(arena_head);
  end = clock();
  printf("   Traversal Time:  %.3f seconds\n",
         (double)(end - start) / CLOCKS_PER_SEC);

  // Sanity check
  if (sum_scatter != sum_arena)
    printf("ERROR: Sums do not match!\n");

  return 0;
}
