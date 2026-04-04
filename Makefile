CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2
TARGET = calc

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.cpp

clean:
	rm -f $(TARGET) temp_input.txt temp_output.txt

.PHONY: all clean