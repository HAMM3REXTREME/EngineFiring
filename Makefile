# Makefile for engine project with source files in src/

CXX = g++
CXXFLAGS = -O2 -Isrc
LDFLAGS = -lsndfile -lportaudio
SRC = src/main.cpp src/Engine.cpp src/AudioVector.cpp src/EngineSoundGenerator.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = engine

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)
