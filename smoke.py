import pyslidingwindow

print(dir(pyslidingwindow))

o = pyslidingwindow.SlidingWindowInt(10)
print(o.max_len())
print(o.length())
try:
    o.get(0)
except RuntimeError:
    print('ok')

o.push(11)
o.push(12)
print(o.max_len() == 10)
print(o.length() == 2)
print(o.get(0)== 11)
print(o.get(-1) == 12)
print(o.first() == 11)
print(o.last() == 12)

o.push(22)
print(o.max_len() == 10)
print(o.length() == 3)
print(o.get(0) == 11)
print(o.get(-1) == 22)

for x in o:
    print(x)

print([x for x in o])

for i in range(100,200):
    o.push(i)
print([x for x in o])
    

# Try it again for float
o = pyslidingwindow.SlidingWindowFloat(10)
print(o.max_len())
print(o.length())
try:
    o.get(0)
except RuntimeError:
    print('ok')

o.push(11.5)
print(o.max_len())
print(o.length())
print(o.get(0))
print(o.get(-1))

o.push(22.5)
print(o.max_len())
print(o.length())
print(o.get(0))
print(o.get(-1))

for x in o:
    print(x)

print([x for x in o])

for i in range(100,200):
    o.push(i+.5)
print([x for x in o])
    
print(o.raw())
