CXXFLAGS = -Wno-deprecated -g -std=c++11 -O2 -D_REENTRANT -D_GNU_SOURCE

INC = -I../../glog/include \
	  -I../../jemalloc/include

SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, %.o, $(SRCS))

Target = libmarco.a

all : $(Target)

%.o : %.cpp
	$(CXX) $(CXXFLAGS) $(INC) -c -o $@ $< 

$(Target) : $(OBJS)
	@$(RM) $(Target)
	ar cru $(Target) $(OBJS)
	@ranlib $(Target)

clean :
	$(RM) $(OBJS) $(Target)

install :
	cp -f *.h ../include/
	mv $(Target) ../

