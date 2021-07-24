CXX=g++-9
WARNINGS=-Wall -Wextra -Wpedantic
ASAN=-fsanitize=address -static-libasan
FLAGS=-O3 -g -std=c++17 $(WARNINGS) -I ../include

all: co_test

co_test: co_sum co_sum4 co_test.cpp
	$(CXX) $(FLAGS) -o co_test co_test.cpp co_sum.o co_sum4.o

co_sum4: co_sum4.cpp co_sum4.hpp
	$(CXX) $(FLAGS) -c co_sum4.cpp

co_sum: co_sum.cpp co_sum.hpp
	$(CXX) $(FLAGS) -c co_sum.cpp

clean:
	rm -rf *.o co_test