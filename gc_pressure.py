import gc
import time
import sys


class LinearNode:
    """A clean citizen. No cycles"""

    def __init__(self):
        self.x = 1


class CyclicNode:
    """A problem child. Creates a reference cycle"""

    def __init__(self):
        self.x = 1
        self.me = self


def stress_test(label, node_class, count, enable_gc=True):
    if enable_gc:
        gc.enable()
    else:
        gc.disable()

    start_time = time.perf_counter()

    for _ in range(count):
        n = node_class()

    end_time = time.perf_counter()
    duration = end_time - start_time

    unreachable = gc.get_count()

    print(f"[{label}]")
    print(f"  Duration: {duration:.4f} seconds")
    print(f"  GC State: {'Enabled' if enable_gc else 'Disabled'}")
    print(f"  Current Gen Counts: {unreachable}")
    print("-" * 40)

    # Force clean for next test
    gc.collect()


print("=== The GC Latency Experiment ===\n")

N = 5_000_000

stress_test("Linear / Ref Counting", LinearNode, N, enable_gc=True)

stress_test("Cyclic / GC Scans", CyclicNode, N, enable_gc=True)

stress_test("Cyclic / GC Disabled", CyclicNode, N, enable_gc=False)
