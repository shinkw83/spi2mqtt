CC=g++

HEADER :=$(wildcard *.h)
SOURCES := $(wildcard *.cpp)
OBJS=$(patsubst %.cpp, %.o,$(SOURCES))

TARGET = libmqttcpp.so

CFLAGS = -std=c++11 -pedantic-errors -Wall -Wno-deprecated -D_LINUX_COMPILE
LIB_PATHS = 
INC_PATHS = -I../../../include -I/usr/local/include

$(TARGET) : $(OBJS)
	@echo ""
	@echo "[OBJECTS] $(OBJS)"
	@echo ""
	$(CC) -shared -Wl,-soname,$(TARGET) $(CFLAGS) $(LIB_PATHS) -o $(TARGET) $(OBJS) $(USING_LIBS)

%.o: %.cpp $(HEADER)
	@echo ""
	@echo "[COMPILE] $@ --------------------------------------"
	$(CC) -fPIC $(CFLAGS) -c $< -o $@ $(LIB_PATHS) $(INC_PATHS)


install: 
	@echo ""
	@echo "[INSTALL] ------------------------------------"
	cp -f $(TARGET) ../../../lib
	cp -f $(TARGET) /usr/local/lib
	cp -f *.h ../../../include
clean:
	@echo ""
	@echo "[CLEAN] --------------------------------------"
	rm -f $(OBJS) $(TARGET)

