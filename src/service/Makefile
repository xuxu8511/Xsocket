CXXFLAGS = -Wall -Wpointer-arith -g -std=c++11 -O2 -D_REENTRANT -D_GNU_SOURCE

DEPS = ../../deps

MARCO = $(DEPS)/marco
GLOG = $(DEPS)/glog
JEMALLOC = $(DEPS)/jemalloc
	
INC = -I$(MARCO)/include\
      -I$(GLOG)/include\
      -I$(JEMALLOC)/include\
      -I../protocol/in\
      -I../protocol/out\
      -I../../deps/protobuf-2.6.1/include \
      -I./

LIB = $(MARCO)/libmarco.a\
      $(GLOG)/lib/libglog.a\
      $(JEMALLOC)/lib/libjemalloc.a\
      ../protocol/in/libinprotocol.a\
      ../protocol/out/liboutprotocol.a\
      ../../deps/protobuf-2.6.1/lib/libprotobuf.a \
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
