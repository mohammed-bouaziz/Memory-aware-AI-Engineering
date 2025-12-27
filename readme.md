# Memory Mechanics: Caches, Allocations & The "Speed Gap"

**A practical exploration of CPU memory hierarchy, cache locality, and allocation strategies.**

This repository contains C implementations designed to demonstrate the physical realities of computer memory. It highlights why theoretical "Big-O" notation often fails to account for real-world hardware constraints like cache lines, memory latency, and `malloc` overhead.

These experiments are part of a broader study path analyzing why high-level structures (like Python Dictionaries) differ in performance compared to low-level contiguous memory (like NumPy Arrays or C-Structs).

## 📂 The Experiments

### 1. Cache Latency & Effective Access Time
**File:** `demistifying-cache.c` (Referred to as the Cache Test)

This program demonstrates the **Memory Hierarchy** by measuring the time it takes to access data at different storage intervals. It calculates the **Effective Access Time (EAT)** to visualize the cost of a "Cache Miss."

* **What it does:** It creates arrays of increasing sizes (fitting in L1, then L2, then L3, then RAM) and performs two types of reads:
    1.  **Sequential Access:** Predictable pattern. The CPU "prefetcher" loads data into the cache before you need it.
    2.  **Random Access:** Unpredictable pattern. The CPU cannot guess the next number, forcing it to fetch data from slower memory (L3 or RAM).
* **Key Insight:** You will observe that as soon as the array size exceeds the L3 cache size (usually ~8MB), performance drops drastically (often by 10x or more).

### 2. Allocation Strategies: Scattered vs. Arena
**File:** `dynamic-allocation.c` (Referred to as the Allocation Test)

This program compares two ways of building a standard Linked List of 10 million nodes to demonstrate **Memory Locality** and **Allocation Overhead**.

* **Strategy A (Scattered/Naive):** Calls `malloc` 10 million times.
    * *Result:* High overhead from system calls. Nodes are scattered randomly in RAM, leading to poor cache locality during traversal.
* **Strategy B (Arena/Pool):** Calls `malloc` exactly once to grab a giant block, then manually assigns pointers within that block.
    * *Result:* Near-instant allocation. Nodes are physically next to each other in RAM, allowing the CPU to read them efficiently.

---

## 🚀 How to Run

### Prerequisites
You need a C compiler (GCC or Clang).

### 1. Run the Cache Latency Test (`demistifying-cache.c`)
```bash
# Compile (using -O2 to optimize, but code prevents over-optimization)
gcc -O2 demistifying-cache.c -o cache_test

# Run
./cache_test
```

### 2. Run the Allocation Strategy Test (`dynamic-allocation.c`)

```bash
# Compile
gcc -O2 dynamic-allocation.c -o arena_test

# Run
./arena_test
```

---

## 📊 Sample Output Analysis

When running the **Cache Test**, you will likely see a jump in nanoseconds per access when the data size exceeds **L3 Cache**:

```text
Array size: 64 MB (exceeds L3, RAM)
  Sequential Access: ~2.1 ns
  Random Access:    ~65.0 ns  <-- The cost of hitting RAM
```

When running the **Allocation Test**:

```text
2. ARENA ALLOCATION
   Allocation Time: 0.001 seconds <-- ~1000x faster than standard malloc loop
```
