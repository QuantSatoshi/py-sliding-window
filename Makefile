.PHONY: test
test: testcxx testpython

.PHONY: testpython
testpython: SO
	python3 test.py

SO: setup.py src/slidingWindowArr.cc src/slidingWindowArr.h src/slidingwindow.cc
	python3 setup.py build_ext --inplace
	touch SO

.PHONY: testcxx
testcxx: cxxtest
	./cxxtest

cxxtest: src/cxxtest.cc src/slidingWindowArr.cc src/slidingWindowArr.h
	$(CXX) -o cxxtest src/cxxtest.cc src/slidingWindowArr.cc

clean:
	rm *.so build cxxtest SO
