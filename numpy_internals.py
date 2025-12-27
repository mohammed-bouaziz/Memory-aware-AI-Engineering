import sys
import time
import numpy as np
import ctypes


def get_memory_address(obj):
    """Helper to get the raw memory address of the data buffer."""
    if isinstance(obj, np.ndarray):
        return obj.ctypes.data
    else:
        return id(obj)


print("=== The Systems Showdown: Python List vs NumPy ===\n")

N = 10_000_000  # 10 Million items

# --- TEST 1: MEMORY FOOTPRINT ---
print(f"1. MEMORY ALLOCATION (N={N:,})")

# Python List
# Note: as we have seen, sys.getsizeof(list) only counts the pointer array, not the objects inside!
# We must sum the overhead of the integers too.
t0 = time.perf_counter()
py_list = [float(i) for i in range(N)]
t1 = time.perf_counter()
list_ptr_size = sys.getsizeof(py_list)
# Each float object is ~24 bytes + 8 byte pointer in the list = 32 bytes/item effective
approx_total_list_mem = list_ptr_size + (N * 24)

print(f"   [Python List]")
print(f"     Creation Time: {t1-t0:.4f}s")
print(f"     RAM Usage:     {approx_total_list_mem / 1024 / 1024:.2f} MB (Estimated)")
print(f"     Structure:     Scattered Pointers -> Boxed Floats")

# NumPy Array
t0 = time.perf_counter()
np_arr = np.arange(N, dtype=np.float64)  # 8 bytes per float
t1 = time.perf_counter()
print(f"   [NumPy Array]")
print(f"     Creation Time: {t1-t0:.4f}s")
print(f"     RAM Usage:     {np_arr.nbytes / 1024 / 1024:.2f} MB (Exact Payload)")
print(f"     Structure:     Single Contiguous Block (C-Array)")
print(f"   -> IMPROVEMENT: {approx_total_list_mem / np_arr.nbytes:.1f}x smaller")


# --- TEST 2: CPU CACHE & VECTORIZATION ---
print(f"\n2. PROCESSING SPEED (Summation)")

# Python Sum
t0 = time.perf_counter()
sum_py = sum(py_list)
t1 = time.perf_counter()
py_time = t1 - t0
print(f"   [Python List] Time: {py_time:.4f}s")

# NumPy Sum
t0 = time.perf_counter()
sum_np = np_arr.sum()
t1 = time.perf_counter()
np_time = t1 - t0
print(f"   [NumPy Array] Time: {np_time:.4f}s")

print(f"   -> SPEEDUP: {py_time / np_time:.1f}x faster")
print("      (Due to Cache Locality + SIMD Instructions)")


# --- TEST 3: STRIDES & ZERO-COPY ---
print(f"\n3. STRIDES & VIEWS (The 1D Tape Theory)")

# We reshape the 1D array into a 2D matrix
original_addr = get_memory_address(np_arr)
matrix = np_arr.reshape((1000, 10000))  # 10M items -> 1000x10000
new_addr = get_memory_address(matrix)

print(f"   Original Array Strides: {np_arr.strides}")
print(f"   Reshaped Matrix Strides: {matrix.strides}")
print(f"   Original Address: {hex(original_addr)}")
print(f"   Reshaped Address: {hex(new_addr)}")

if original_addr == new_addr:
    print("   -> PROOF: Addresses Match! No memory was moved.")
    print("             Reshaping is just math on the 'Strides' tuple.")
else:
    print("   -> MEMORY COPIED.")
