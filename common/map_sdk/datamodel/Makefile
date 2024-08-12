CXX=g++
CC=gcc
CXXFLAGS+= -std=c++11 -I./include
AR=ar

ifeq ($(debug),1)
	CXXFLAGS+=-g -pg -std=c++11 -I./include
	DB_LIB=libdatamodel_d
else
	CXXFLAGS+=-fPIC -O2 -std=c++11 -I./include
	DB_LIB=libdatamodel
endif

HOME_DIR=./
DB_DIR=../devel/bin
OBJ_DIR=../devel/obj/datamodel

DB_SOURCE=$(wildcard src/common/*.cpp) $(wildcard src/spatialindex/*.cpp) $(wildcard src/structs/*cpp)

OBJS=$(DB_SOURCE:.cpp=.o)

$(DB_DIR)/$(DB_LIB).a : $(OBJS)
	$(AR) -r $@ $(OBJS)
	-rm -rf $(OBJS)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(DB_DIR):
	mkdir -p $(DB_DIR)

.PHONY : clean
clean :
	-rm -rf $(OBJS)
	-rm -rf $(DB_DIR)/$(DB_LIB).a
