EXE := morph

SRC_DIR := MorphingUsingLevelSets
OBJ_DIR := build

SRCS := $(shell find $(SRC_DIR) -name *.cpp)
OBJS := $(SRCS:%=$(OBJ_DIR)/%.o)

FLAGS := 
CC := g++
LDFLAGS := -lopenvdb -ltbb -lHalf -lboost_iostreams

.PHONY: clean

$(EXE): $(OBJS)
	$(CC) $(FLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(OBJ_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm $(EXE) $(OBJS)
