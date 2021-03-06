BIN += triangle

#? Our files
SRC += $(wildcard src/*.cpp)
SRC += $(wildcard src/*/*.cpp)
SRC += $(wildcard src/*/*/*.cpp)
INC += -I include/ -I external/glm/

SHR += $(wildcard src/Shaders/*.vert)
SHR += $(wildcard src/Shaders/*/*.vert)
SHR += $(wildcard src/Shaders/*.frag)
SHR += $(wildcard src/Shaders/*/*.frag)
# INC += $(addprefix -I ,$(wildcard includes/**))

#? Additional settings
FLG += -std=c++17 -g -O2 -m64
# ERR += -Wall -Wextra -Werror

LIB += -lm
LIB +=
# FLG += -Wall -Wextra -Werror
# FRM += -framework OpenGL

#? Make variables
OBJ = $(SRC:.cpp=.o)
DEP = $(SRC:.cpp=.d)

SPV += $(addsuffix .spv, $(SHR))

#? OS settings
#? Thank you, stackoverflow
ifeq ($(OS),Windows_NT) 
    detected_OS = Windows
else
    detected_OS = $(shell sh -c 'uname 2>/dev/null || echo Unknown')
endif

ifeq ($(detected_OS),Windows)
	FLG += -D WINDOWS
	EXT += .exe
	INC += -I external/win32/glfw/include/ -I C:\VulkanSDK\1.2.135.0\Include
	LIB += -L external/win32/glfw/lib-mingw-w64 -lglfw3dll
	LIB += -L C:\VulkanSDK\1.2.135.0\Lib -lvulkan-1
endif
ifeq ($(detected_OS),Linux)
	FLG += -D LINUX
	EXT += .out
endif
ifeq ($(detected_OS),Darwin)
	FLG += -D OSX
	EXT += .out
endif

all: binclean debug
	@echo "Debug build complete."

release: fclean $(BIN)
	@echo "Release build complete."

-include $(DEP)

%.o: %.cpp
	@g++ $(FLG) -MMD -MP -c $< -o $@ $(INC)

%.frag.spv: %.frag
	@glslc $< -o $@ -Werror

%.vert.spv: %.vert
	@glslc $< -o $@ -Werror

$(BIN): $(OBJ) $(SPV)
	@g++ $(FLG) -o $(BIN)$(EXT) $(OBJ) $(LIB) $(FRM)

debug: FLG += -D DEBUG -g
debug: FLG += -Wall -Wextra -Werror
debug: $(BIN)

clean:
	@rm -f $(OBJ)
	@rm -f $(DEP)
	@rm -f $(SPV)

fclean: clean
	@rm -f $(BIN)

binclean:
	@rm -f $(BIN)

re: fclean all