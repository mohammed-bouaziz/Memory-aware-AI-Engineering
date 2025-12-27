import numpy as np
import time
import os
import psutil

# CONSTANTS
FILENAME = "massive_data.bin"
N_VECTORS = 10_000_000
DIM = 128
DTYPE = np.float32


def print_memory(label):
    process = psutil.Process(os.getpid())
    mem_info = process.memory_info()
    print(f"[{label}] RAM Used: {mem_info.rss / 1024 / 1024:.2f} MB")


def create_dummy_file():
    """Writes raw bytes to disk. No RAM overhead."""
    print(f"Creating {FILENAME}...")
    expected_size_gb = (N_VECTORS * DIM * 4) / (1024**3)
    print(f"Expected File Size: {expected_size_gb:.2f} GB")

    mm = np.memmap(FILENAME, dtype=DTYPE, mode="w+", shape=(N_VECTORS, DIM))

    mm[0] = 1.0
    mm[N_VECTORS - 1] = 99.0

    mm.flush()
    print("File created on disk.")
    del mm


def read_mapped_file():
    print("\n--- Benchmarking Memory Map ---")
    print_memory("Before Mapping")

    t0 = time.time()
    data = np.memmap(FILENAME, dtype=DTYPE, mode="r", shape=(N_VECTORS, DIM))
    dt = time.time() - t0

    print(f"Mapping Time: {dt:.6f}s (Instant!)")
    print_memory("After Mapping (Should be near zero change)")

    print("\nReading random vector...")
    t0 = time.time()
    vec = data[N_VECTORS - 1]
    dt = time.time() - t0
    print(f"Read Time (Cold): {dt:.6f}s (Disk Latency)")
    print(f"Value Verification: {vec[0]} (Should be 99.0)")

    print("\nReading same vector again...")
    t0 = time.time()
    vec = data[N_VECTORS - 1]
    dt = time.time() - t0
    print(f"Read Time (Hot):  {dt:.6f}s (RAM Speed - OS cached the page)")

    del data


if __name__ == "__main__":
    create_dummy_file()
    read_mapped_file()
