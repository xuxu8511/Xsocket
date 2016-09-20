CXXFLAGS = -Wall -Wpointer-arith -O2 -pipe -D_REENTRANT -D_GNU_SOURCE

PROTOC = ../../../deps/protobuf-2.6.1/bin/protoc

INC = -I./ -I../../../deps/protobuf-2.6.1/include

LIB_TARGET = liboutprotocol.a

PROTO = cg.proto
.SECONDARY:$(PROTO)

SRC = $(patsubst %.proto,%.pb.cc,$(PROTO))
.SECONDARY:$(SRC)

HR = $(patsubst %.cc,%.h,$(SRC))

OBJ = $(patsubst %.cc,%.o,$(SRC))

all : $(LIB_TARGET) 

%.proto : ../../../../common/out_protocol/%.proto
	cp --preserve $< ./

%.pb.cc : %.proto
	$(PROTOC) --cpp_out=./ $<

%.pb.o : %.pb.cc 
	$(CXX) $(CXXFLAGS) $(INC) -c -o $@ $<

$(LIB_TARGET) : $(OBJ)
	@$(RM) $(LIB_TARGET)
	ar cru $(LIB_TARGET) $(OBJ)
	@ranlib $(LIB_TARGET)

clean :
	$(RM) $(HR) $(SRC) $(OBJ) $(LIB_TARGET)

