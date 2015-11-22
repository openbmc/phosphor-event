TEST      = listfunc
OBJS_TEST = $(TEST).o

EXE     = event_messaged

OBJS    = $(EXE).o   \
          list.o 	 \


DEPPKGS = libsystemd
CC      ?= $(CROSS_COMPILE)gcc
INCLUDES += $(shell pkg-config --cflags $(DEPPKGS)) 
LIBS += $(shell pkg-config --libs $(DEPPKGS))

all: $(EXE)

%.o : %.c 
	$(CC) -c $< $(CFLAGS) $(INCLUDES) -o $@

$(EXE): $(OBJS)
	$(CC) $^ $(LDFLAGS) $(LIBS) -o $@

$(TEST): $(OBJS_TEST)
	$(CC) $^ $(LDFLAGS) $(LIBS) -o $@

clean:
	rm -f $(OBJS) $(EXE) *.o *.d
