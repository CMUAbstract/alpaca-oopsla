LIB = libalpaca

include ../Makefile.config

OBJECTS = \
	alpaca.o \

DEPS +=  \
	libmsp \

override SRC_ROOT = ../../src

override CFLAGS += \
	-DALPACA \
	-I$(SRC_ROOT)/include \
	-I$(SRC_ROOT)/include/libalpaca \

