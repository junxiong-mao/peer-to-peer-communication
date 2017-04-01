COMPILER = g++

MAIN = client.cc
OTHERSRCS = client.cpp file.cpp torrent.cpp piece.cpp
OUTLIB = ../socket.o ../util.o

CFLAGS = -g -Wall
SRCS = $(MAIN) $(OTHERSRCS)
OBJS = $(SRCS:.cpp=.o) $(OUTLIB)

TARGET = $(MAIN:.cc=)

%.o: %.cpp
	$(COMPILER) $(CFLAGS) -o $@ -c $<

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(COMPILER) $(OBJS) $(CFLAGS) -o $(TARGET) -lpthread -lcrypto

clean:
	rm -f $(TARGET) $(OTHERSRCS:.cpp=.o)
