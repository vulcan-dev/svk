# VULKAN_ROOT (1.3.250.1)
VULKAN_ROOT = F:/SDK/Vulkan/1.3.250.1

LIB_DIRS := -Ldeps/sdl2/lib -Ldeps/cglm/lib -L$(VULKAN_ROOT)/Lib
INC_DIRS := -Ideps/sdl2/include -Ideps/cglm/include -I$(VULKAN_ROOT)/Include -Icode/include

SVK_SOURCES := $(wildcard code/src/svk/*.c)
SVK_ENGINE_SOURCES := $(wildcard code/src/svk/engine/*.c)
SVK_ENGINE_UTIL_SOURCES := $(wildcard code/src/svk/util/*.c)
SVK_ENGINE_CAMERA_SOURCES = $(wildcard code/src/svk/camera/*.c)
    SRC := $(SVK_SOURCES) $(SVK_ENGINE_SOURCES) $(SVK_ENGINE_UTIL_SOURCES) $(SVK_ENGINE_CAMERA_SOURCES) code/src/main.c

CC = clang
CFLAGS = -std=c11 -g -O0 -fsanitize=address -fno-omit-frame-pointer -Wno-void-pointer-to-int-cast $(INC_DIRS) -luser32
LDFLAGS = $(LIB_DIRS) -lSDL2main -lSDL2 -lvulkan-1
TARGET = build/svk.exe

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)
