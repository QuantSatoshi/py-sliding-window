.PHONY: test
test: testcxx testpython

.PHONY: testpython
testpython: SO smoke.py unittesting.py
	python3 smoke.py
	python3 unittesting.py

.PHONY: SO
SO: setup.py src/slidingWindowArr.cc src/slidingWindowArr.h src/slidingwindow.cc
	python3 setup.py build_ext --inplace
	touch SO

.PHONY: testcxx
testcxx: cxxtest
	./cxxtest

cxxtest: src/cxxtest.cc src/slidingWindowArr.cc src/slidingWindowArr.h
	$(CXX) -o cxxtest src/cxxtest.cc src/slidingWindowArr.cc

clean:
	rm -rf *.so build cxxtest SO

format:
	isort .
	black .

dev-install:
	pip install black==20.8b1
	pip install isort==5.6.4
	