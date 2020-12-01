SIMULATOR = simulator

SIMULATOR_SOURCES := $(wildcard src/*.cpp)
SIMULATOR_OBJS := $(SIMULATOR_SOURCES:%.cpp=%.o)

JSON_LIB=libjsoncpp.a

CPPFLAGS= -std=c++11 -g

ifeq ($(BUILD),debug)
	#let's just always include debugging information
	#CPPFLAGS += -g
else
	CPPFLAGS += -O2
endif

%.o: %.cpp Makefile
	$(CXX) $(CPPFLAGS) -c  -I include/ $< -o $@

all: $(SIMULATOR)

debug:
	make "BUILD=debug"

$(SIMULATOR): $(SIMULATOR_OBJS)
	$(CXX) $(CPPFLAGS)  $^ -o $@ libjsoncpp.a -I include/

clean:
	rm -rf *~ src/*.o $(SIMULATOR) *.out
