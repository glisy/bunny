SRC += test.c
SRC += $(wildcard deps/*/*.c)

CFLAGS += -I deps
CFLAGS += -framework OpenGL
CFLAGS += -l glfw3

test: $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o test
