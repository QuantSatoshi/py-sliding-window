# py-sliding-window

 pip install git+https://github.com/quantsatoshi/py-sliding-window.git#main
 
 ### usage
```
import pyslidingwindow

slidingwindow = pyslidingwindow.SlidingWindowFloat(3)
slidingwindow.push(1).push(5)
print(slidingwindow.max_len())
print(slidingwindow.length())

print(slidingwindow.max_len() == 3)
print(slidingwindow.length() == 2)
print(slidingwindow.get(0) == 1)
print(slidingwindow.get(-1) == 5)
print(slidingwindow.last() == 5)
print(slidingwindow.first() == 1)
print(slidingwindow.is_full() == False)
print(sum(slidingwindow) == 6)
# all the above case should return true

# get raw numpy array
slidingwindow.push(7).push(9)
np_raw = slidingwindow.raw() # returns numpy raw array (unordered)
print(np_raw) # [9. 5. 7.]

# get list ordered array
ordered = list(slidingwindow)
print(ordered) # [5.0, 7.0, 9.0]
```
