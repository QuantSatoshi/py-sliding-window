#include "slidingWindowArr.h"

#include <iostream>
#include <sstream>

#define CHECK_TRUE(ok,msg) do { if(!(ok)) { std::stringstream ss; ss << __FILE__ << ':' << __LINE__ << ": " << msg; throw std::runtime_error(msg); } } while(0);
#define CHECK_EXPRESSION(ok) CHECK_TRUE(ok,#ok)

template<class T>
void test(const char* type)
{
  std::cout << "Testing " << type << '\n';

  bool ok = false;
  try {
    SlidingWindowArr<T> bad(-1);
  } catch(const std::exception&) {
    ok = true;
  }
  CHECK_TRUE(ok,"Mishandled creating array with negative size");

  ok = false;
  try {
    SlidingWindowArr<T> bad(0);
  } catch(const std::exception&) {
    ok = true;
  }
  CHECK_TRUE(ok,"Mishandled creating array with size zero");

  const int N = 20;
  SlidingWindowArr<T> array(N);
  CHECK_EXPRESSION(array.getLength() == 0);
  CHECK_EXPRESSION(array.getMaxLen() == N);

  ok = false;
  int x;
  try {
    x = array.get(0);
  } catch(const std::exception&) {
    ok = true;
  }
  CHECK_TRUE(ok,"Mishandled get() from empty array");

  // Put a value in
  array.push(11);
  CHECK_EXPRESSION(array.getLength() == 1);
  CHECK_EXPRESSION(array.getMaxLen() == N);
  ok = true;
  try {
    x = array.get(0);
  } catch(const std::exception&) {
    ok = false;
  }
  CHECK_TRUE(ok,"Mishandled get() from size 1 array");
  CHECK_EXPRESSION(x == 11);

  x = 0;
  ok = true;
  try {
    x = array.get(-1);
  } catch(const std::exception&) {
    ok = false;
  }
  CHECK_TRUE(ok,"Mishandled get(negative) from size 1 array");
  CHECK_EXPRESSION(x == 11);
  

  ok = false;
  try {
    x = array.get(1);
  } catch(const std::exception&) {
    ok = true;
  }
  CHECK_TRUE(ok,"Mishandled get(out of bounds) from size 1 array");

  

}


int main(void)
{
  test<int>("int");
  test<float>("float");
  test<double>("double");
  return 0;
}
