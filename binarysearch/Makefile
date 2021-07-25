CXX=g++-10
WARNINGS=-Wall -Wextra -Wpedantic
ASAN=-fsanitize=address -static-libasan
FLAGS=-O3 -std=c++17 $(WARNINGS) -I ../include

all: coro_binary_test

coro_binary_test: coro_binary_test.cpp coro_binary.hpp coro_infra.hpp
	$(CXX) $(FLAGS) -o coro_binary_test coro_binary_test.cpp

clean:
	rm -rf coro_binary_test