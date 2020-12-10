# py-sliding-window

 python -m pip install git+https://github.com/quantsatoshi/py-sliding-window.git#main
 
 ```
 import slidingwindow

 o = slidingwindow.SlidingWindowFloat(10)
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
print(o.last())
print(o.first())
```
