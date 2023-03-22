CXX=g++
CXXFLAGS=-Wall -Wextra -pedantic -std=c++20
LDFLAGS=-pthread
OBJECTS=ipkcpc.o ipkcpc_functions.o
TARGET=ipkcpc

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

ipkcpc.o: ipkcpc.cpp ipkcpc_functions.h
	$(CXX) $(CXXFLAGS) -c ipkcpc.cpp

ipkcpc_functions.o: ipkcpc_functions.cpp ipkcpc_functions.h
	$(CXX) $(CXXFLAGS) -c ipkcpc_functions.cpp

clean:
	rm -f $(OBJECTS) $(TARGET)