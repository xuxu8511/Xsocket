ROOT_DIR = $(PWD)
TARGET_DIR = $(ROOT_DIR)/lib
BUILD_STR = `date +%Y%m%d`" build"`cat $(ROOT_DIR)/.build`
JOB_NUM = "-j4"

.PHONY : all clean glog jemalloc marco

all: glog jemalloc marco

clean :
	cd glog && make clean && rm Makefile
	cd jemalloc && make clean && rm Makefile
	cd marco && make clean

glog :
	cd glog && \
		if [ ! -f Makefile ]; then \
		./configure --prefix=$(shell pwd)/glog; \
		fi && \
		make && make install

jemalloc :
	cd jemalloc && \
		if [ ! -f Makefile ]; then \
		./configure --prefix=$(shell pwd)/jemalloc; \
		fi && \
		make

marco :
	cd marco && make
