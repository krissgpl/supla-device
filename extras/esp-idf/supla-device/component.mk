COMPONENT_ADD_INCLUDEDIRS := ../../../src
#COMPONENT_OBJS := file1.o file2.o thing/filea.o thing/fileb.o anotherthing/main.o

COMPONENT_SRCDIRS := ../../../src/supla-common \
										 ../../../src/supla \
										 ../../../src/supla/conditions \
										 ../../../src/supla/clock \
										 ../../../src/supla/control \
										 ../../../src/supla/storage \
										 ../../../src/ \
										 ../../../src/supla/network

CFLAGS += -DSUPLA_DEVICE
CPPFLAGS += -DSUPLA_DEVICE
