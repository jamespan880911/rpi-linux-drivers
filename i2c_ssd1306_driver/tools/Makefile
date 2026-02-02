CC = gcc
CFLAGS = -Wall

TARGETS = test_app ssd1306_writer clear

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS)
