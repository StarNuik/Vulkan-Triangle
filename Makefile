BIN += triangle

#? Our files
SRC += $(wildcard src/*.cpp)
SRC += $(wildcard src/*/*.cpp)
SRC += $(wildcard src/*/*/*.cpp)
INC += -I include/ -I external/glm/
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

all: binclean $(BIN)
	@echo "Compilation successful!"

-include $(DEP)

%.o: %.cpp
	@g++ $(FLG) -MMD -MP -c $< -o $@ $(INC)

$(BIN): $(OBJ)
	@g++ $(FLG) -o $(BIN)$(EXT) $(OBJ) $(LIB) $(FRM)

debug: FLG += -D DEBUG -g
debug: FLG += -Wall -Wextra -Werror
debug: clean $(BIN)

clean:
	@rm -f $(OBJ)
	@rm -f $(DEP)

fclean: clean
	@rm -f $(BIN)

binclean:
	@rm -f $(BIN)

re: fclean all