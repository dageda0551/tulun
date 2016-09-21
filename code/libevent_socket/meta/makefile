
include $(MAKEROOT)/env.mk

SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,%.o,$(SRCS))

%.o : %.cpp
	${CC} ${CXXFLAGS} -c $< -o ./$@

.PHONY : all
all : $(OBJS)
	g++ -o $(MAKEROOT)/metamgr ../obj/*.o metamgr.o -levent -Wno-write-strings -lstdc++
	g++ -o $(MAKEROOT)/metacli ../obj/*.o metacli.o -levent

