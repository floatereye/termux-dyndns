CC = gcc
CFLAGS = -Wall -O2
LIBS = -lcurl -ljson-c
TARGET = dyndns
SRC = dyndns.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET)

install:
	cp $(TARGET) /data/data/com.termux/files/usr/bin/

uninstall:
	rm -f /data/data/com.termux/files/usr/bin/$(TARGET)
