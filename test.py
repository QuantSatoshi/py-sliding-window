import slidingwindow

print(dir(slidingwindow))

o = slidingwindow.SlidingWindowInt(10)
print(o.getMaxLen())
print(o.getLength())
try:
    o.get(0)
except RuntimeError:
    print('ok')

o.push(11)
print(o.getMaxLen())
print(o.getLength())
print(o.get(0))
print(o.get(-1))

o.push(22)
print(o.getMaxLen())
print(o.getLength())
print(o.get(0))
print(o.get(-1))

for x in o:
    print(x)

print([x for x in o])

for i in range(100,200):
    o.push(i)
print([x for x in o])
    

# Try it again for float
o = slidingwindow.SlidingWindowFloat(10)
print(o.getMaxLen())
print(o.getLength())
try:
    o.get(0)
except RuntimeError:
    print('ok')

o.push(11.5)
print(o.getMaxLen())
print(o.getLength())
print(o.get(0))
print(o.get(-1))

o.push(22.5)
print(o.getMaxLen())
print(o.getLength())
print(o.get(0))
print(o.get(-1))

for x in o:
    print(x)

print([x for x in o])

for i in range(100,200):
    o.push(i+.5)
print([x for x in o])
    



