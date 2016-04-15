CXXFLAGS = -Wall -Wpointer-arith -g -std=c++11 -O2 -D_REENTRANT -D_GNU_SOURCE

DEPS = ../../deps

MARCO = $(DEPS)/marco
GLOG = $(DEPS)/glog
JEMALLOC = $(DEPS)/jemalloc

INC  = -I$(MARCO)/include\
	   -I$(GLOG)/include\
	   -I$(JEMALLOC)/include\
	   -I./

LIB = $(MARCO)/libmarco.a\
	  $(GLOG)/lib/libglog.a\
	  $(JEMALLOC)/lib/libjemalloc.a\
	  -lpthread -lrt -ldl
	
SRC = $(wildcard *.cpp)
OBJS = $(SRC:.cpp=.o)

Target = service

all : $(Target)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC) -c -o $@ $<

$(Target): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(CXXFLAGS) $(INC) $(LIB)
	mv $(Target) ../../bin

clean:     
	$(RM) $(OBJS) $(Target)
