# EngineFiring
CXX = clang++
CXXFLAGS = -O2 -Isrc -g
LDFLAGS = -lsndfile -lportaudio -lsfml-graphics -lsfml-window -lsfml-system
SRC = src/main.cpp src/Engine.cpp src/AudioVector.cpp src/EngineSoundGenerator.cpp src/Car.cpp
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
