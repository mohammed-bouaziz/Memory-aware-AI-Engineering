import sys

# The size of an empty object
print(f"Size of an empty object: {sys.getsizeof(object())} bytes")

# In C, an integer is 4 bytes, you will see that it not the case here
x = 1
print(f"Size of int(1): {sys,sys.getsizeof(x)} bytes")

# A list is a PyVarObject (Size + Allocated + Items Pointer)
l = []
print(f"Size of empty list: {sys.getsizeof(l)} bytes")

a = 100
b = 100
print(f"Is a the same memory adress as b ? {a is b}")  # True, it was cached

c = 1000000
d = 1000000
print(f"Is c the same memory adress as d ? {c is d}")  # False, new malloc
