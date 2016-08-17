TARGET=logtail
LIBS=
CC=gcc
CFLAGS=-Wall -D_FORTIFY_SOURCE=2 -g -fPIE -fstack-protector -fPIE -pie -Wl,-z,now -Wl,-z,now
LDFLAGS=-Wl,-Bsymbolic-functions -fPIE -pie -Wl,-z,relro -Wl,-z,now

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	    $(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS)  -c $< -o $@

$(TARGET): $(OBJECTS)
	    $(CC) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $@
		hardening-check -f $@

clean:
	    -rm -f *.o
		-rm -f $(TARGET)
