# VULKAN_ROOT (1.3.250.1)
VULKAN_ROOT = F:/SDK/Vulkan/1.3.250.1

SVK_SOURCES := $(wildcard code/src/svk/*.c)
SVK_ENGINE_SOURCES := $(wildcard code/src/svk/engine/*.c)
SVK_ENGINE_UTIL_SOURCES := $(wildcard code/src/svk/util/*.c)
SRC := $(SVK_SOURCES) $(SVK_ENGINE_SOURCES) $(SVK_ENGINE_UTIL_SOURCES) code/src/main.c

CC = clang
CFLAGS = -std=c11 -g -Wno-void-pointer-to-int-cast -Ideps/sdl2/include -I$(VULKAN_ROOT)/Include -Icode/include -luser32
LDFLAGS = -Ldeps/sdl2/lib -L$(VULKAN_ROOT)/Lib -lSDL2main -lSDL2 -lvulkan-1
TARGET = build/svk.exe

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)
