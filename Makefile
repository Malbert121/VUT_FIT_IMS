CXX = g++
CXXFLAGS = -std=c++11

SRC_BEFORE = beforeImprovement.cpp
OUT_BEFORE = beforeImprovement

SRC_AFTER = afterImprovement.cpp
OUT_AFTER = afterImprovement

LIBS = -lsimlib

all: $(OUT_BEFORE) $(OUT_AFTER)

$(OUT_BEFORE): $(SRC_BEFORE)
	$(CXX) $(CXXFLAGS) $(SRC_BEFORE) -o $(OUT_BEFORE) $(LIBS)

$(OUT_AFTER): $(SRC_AFTER)
	$(CXX) $(CXXFLAGS) $(SRC_AFTER) -o $(OUT_AFTER) $(LIBS)

clean:
	rm -f $(OUT_BEFORE) $(OUT_AFTER)

.PHONY: all clean

