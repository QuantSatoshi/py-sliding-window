#ifndef SLIDING_WINDOW_ARR_H
#define SLIDING_WINDOW_ARR_H
#include <vector>

template <class T>
class SlidingWindowArr
{
public:
  SlidingWindowArr(int maxLen);
  ~SlidingWindowArr();
  int getLength() const;
  int getMaxLen() const;

  void push(T value);
  T get(int index) const;
  T first() const;
  T last() const;

  // return the ordered array so we can iterate through them
  const T *toArr() const;
  T *toArr();

  // return raw unordered array. (faster). We can use this to calculate some metric like mean, std, etc.
  const T *toUnorderedArr() const;
  T *toUnorderedArr();

  // check if date is equal to window size
  bool isFull() const;

private:
  int _dataLen;
  int _maxLen;
  int _cursor;
  T *_historyValues;
};

#endif
