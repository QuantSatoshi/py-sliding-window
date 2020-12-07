import slidingwindow

print('SMOKE:',slidingwindow)

print(dir(slidingwindow))

W = slidingwindow.SlidingWindow(10,3)
print(W)
for i in range(1,11):
    W.insert(11*i)

for i in range(1,11):
    print(W.get(0), W.get(1), W.get(2))
    W.slide()

# and we can get errors
W.get(1000)
