CXX=g++
CC=gcc
AR=ar

ifeq ($(debug),1)
	CXXFLAGS+=-g -pg -std=c++11 -I./include -I../datamodel/include -I../mapdb/include -I../map_engine/include -I../route_plan/include -I../transmission/include
	DB_LIB=libroutingmap_d
else
	CXXFLAGS+=-fPIC -O2 -std=c++11 -I./include -I../datamodel/include -I../mapdb/include -I../map_engine/include -I../route_plan/include -I../transmission/include
	DB_LIB=libroutingmap
endif

HOME_DIR=./
DB_DIR=../devel/bin

DB_SOURCE=$(wildcard src/*.cpp)

OBJS=$(DB_SOURCE:.cpp=.o) 

$(DB_DIR)/$(DB_LIB).a : $(OBJS)
	$(AR) -r $@ $(OBJS)
	-rm -rf $(OBJS)
	
$(DB_DIR):
	mkdir -p $(DB_DIR)
	
.PHONY : clean
clean :
	-rm -rf $(OBJS)
	-rm -rf $(DB_DIR)/$(DB_LIB).a 
