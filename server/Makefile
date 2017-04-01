COMPILER = g++
SERVERMAIN = server.cpp
SERVEROTHER = server_lib.cpp
OUTLIB = ../socket.o ../util.o

CFLAGS = -g -Wall
SERVERSRCS = $(SERVERMAIN) $(SERVEROTHER)

SERVEROBJS = $(SERVERSRCS:.cpp=.o) $(OUTLIB)
 
SERVERTARGET = $(SERVERMAIN:.cpp=)

%.o: %.cpp
	$(COMPILER) $(CFLAGS) -o $@ -c $<

all: $(SERVERTARGET)

$(SERVERTARGET): $(SERVEROBJS)
	$(COMPILER) $(SERVEROBJS) $(CFLAGS) -o $(SERVERTARGET) -lpthread -lcrypto

clean:
	rm -f $(SERVEROBJS) $(SERVERTARGET)
