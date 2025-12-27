from memory_profiler import profile
import numpy as np
import time
import sys
import gc

N_VECTORS = 1_000_000
DIM = 128


@profile
def benchmark_python_dict():
    print(f"\n--- 1. Python Dictionary of Lists ---")
    print(f"Allocating {N_VECTORS} vectors (dim={DIM})...")

    data = {}
    t0 = time.time()

    for i in range(N_VECTORS):
        vec = [0.5] * DIM
        data[i] = vec

    dt = time.time() - t0
    print(f"Creation Time: {dt:.2f}s")

    t0 = time.time()
    _ = data[500_000]
    dt = time.time() - t0
    print(f"Random Access (id=500k): {dt:.6f}s")

    print("Simulating Batch Dot Product...")
    t0 = time.time()
    total = 0.0
    for i in range(10_000):
        vec = data[i]
        for val in vec:
            total += val
    dt = time.time() - t0
    print(f"Summing first 10k vectors: {dt:.2f}s")

    return data


@profile
def benchmark_numpy():
    print(f"\n--- 2. NumPy Contiguous Matrix ---")
    print(f"Allocating {N_VECTORS}x{DIM} matrix...")

    t0 = time.time()
    data = np.full((N_VECTORS, DIM), 0.5, dtype=np.float32)
    dt = time.time() - t0
    print(f"Creation Time: {dt:.2f}s")

    t0 = time.time()
    _ = data[500_000]  # View creation (Strides logic from Step 8)
    dt = time.time() - t0
    print(f"Random Access (id=500k): {dt:.6f}s")

    print("Simulating Batch Dot Product...")
    t0 = time.time()
    _ = data[:10000].sum()
    dt = time.time() - t0
    print(f"Summing first 10k vectors: {dt:.2f}s")

    return data


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "numpy":
        benchmark_numpy()
    else:
        benchmark_python_dict()
