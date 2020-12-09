.PHONY: test
test: testcxx testpython

.PHONY: testpython
testpython: SO
	python3 test.py

SO: setup.py slidingWindowArr.cc slidingWindowArr.h slidingwindow.cc
	python3 setup.py build_ext --inplace
	touch SO

.PHONY: testcxx
testcxx: cxxtest
	./cxxtest

cxxtest: cxxtest.cc slidingWindowArr.cc slidingWindowArr.h
	$(CXX) -o cxxtest cxxtest.cc slidingWindowArr.cc
