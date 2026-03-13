CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude
SRCS     = src/main.cpp src/Task.cpp src/TaskManager.cpp src/CLI.cpp
TARGET   = task_manager

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)

.PHONY: all clean
