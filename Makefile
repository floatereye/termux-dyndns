CC = clang  # Using clang, the default compiler for Termux (based on LLVM)
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
	install -m 755 $(TARGET) /data/data/com.termux/files/usr/bin/

uninstall:
	rm -f /data/data/com.termux/files/usr/bin/$(TARGET)
