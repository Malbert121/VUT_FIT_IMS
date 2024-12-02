# Define the compiler and flags
CXX = g++
CXXFLAGS = -std=c++11

# Define the source files and output binaries
SRC_BEFORE = beforeImprovement.cpp
OUT_BEFORE = simulation_before

SRC_AFTER = afterImprovement.cpp
OUT_AFTER = simulation_after

# Define the library to link with
LIBS = -lsimlib

# Default target to build both executables
all: $(OUT_BEFORE) $(OUT_AFTER)

# Rule to build the "before improvement" version
$(OUT_BEFORE): $(SRC_BEFORE)
	$(CXX) $(CXXFLAGS) $(SRC_BEFORE) -o $(OUT_BEFORE) $(LIBS)

# Rule to build the "after improvement" version
$(OUT_AFTER): $(SRC_AFTER)
	$(CXX) $(CXXFLAGS) $(SRC_AFTER) -o $(OUT_AFTER) $(LIBS)

# Clean up the build files
clean:
	rm -f $(OUT_BEFORE) $(OUT_AFTER)

.PHONY: all clean

