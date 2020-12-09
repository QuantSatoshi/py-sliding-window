#include "slidingWindowArr.h"
#include <stdexcept>
#include <sstream>

template <class T>
SlidingWindowArr<T>::SlidingWindowArr(int maxLen)
  : _dataLen(0),
    _maxLen(maxLen),
    _cursor(0),
    _historyValues(nullptr)
{
  if (_maxLen <= 0) {
    throw std::runtime_error("maxLen must be positive");
  }
  _historyValues = new T[_maxLen]();
}

template <class T>
SlidingWindowArr<T>::~SlidingWindowArr()
{
  delete[] _historyValues;
}

template <class T>
int SlidingWindowArr<T>::getLength() const
{
  return _dataLen;
}

template <class T>
int SlidingWindowArr<T>::getMaxLen() const
{
  return _maxLen;
}

template <class T>
void SlidingWindowArr<T>::push(T value)
{
  if (_dataLen < _maxLen)
  {
    _historyValues[_dataLen] = value;
    _dataLen++;
  }
  else
  {
    _historyValues[_cursor % _maxLen] = value;
    _cursor++;
  }
}

template <class T>
T SlidingWindowArr<T>::get(int index) const
{
  if (index < 0)
  {
    index += _dataLen;
  }
  if (index < 0 || index >= _dataLen)
  {
    std::stringstream ss;
    ss
      << "index "
      << index
      << " out of range 0:"
      << _dataLen;

    throw std::runtime_error(ss.str());
  }
  return _historyValues[(_cursor + index) % _maxLen];
}

template <class T>
T SlidingWindowArr<T>::first() const
{
  return get(0);
}

template <class T>
T SlidingWindowArr<T>::last() const
{
  return get(this->_dataLen-1);
}

template <class T>
const T* SlidingWindowArr<T>::toArr() const {
  return _historyValues + _cursor%_dataLen;
}

template <class T>
T* SlidingWindowArr<T>::toArr() {
  return _historyValues + _cursor%_dataLen;
}

template <class T>
const T* SlidingWindowArr<T>::toUnorderedArr() const {
  return _historyValues;
}

template <class T>
T* SlidingWindowArr<T>::toUnorderedArr() {
  return _historyValues;
}

template class SlidingWindowArr<int>;
template class SlidingWindowArr<float>;
template class SlidingWindowArr<double>;
