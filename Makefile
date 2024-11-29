# Define the compiler and flags
CXX = g++
CXXFLAGS = -std=c++11

# Define the source file and the output binary
SRC = noTransport.cpp
OUT = noTransport

# Define the library to link with
LIBS = -lsimlib

# The rule to build the project
$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LIBS)

# Clean up the build files
clean:
	rm -f $(OUT)

.PHONY: clean
