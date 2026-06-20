OS := $(shell uname)

ifeq ($(OS), Darwin)

GLFW_LIB := $(shell brew --prefix glfw 2>/dev/null)/lib

# Arquivos fonte da SoLoud, coletados nos mesmos diretórios usados pelo
# CMakeLists.txt (core, audiosource/wav e backend miniaudio).
SOLOUD_SRC := $(wildcard soloud/src/core/*.cpp) \
              $(wildcard soloud/src/audiosource/wav/*.cpp) \
              $(wildcard soloud/src/audiosource/wav/*.c) \
              $(wildcard soloud/src/backend/miniaudio/*.cpp)

SRC := src/main.cpp src/correcao.cpp src/glad.c src/textrendering.cpp \
       src/tiny_obj_loader.cpp src/stb_image.cpp $(SOLOUD_SRC)

./bin/macOS/main: $(SRC) include/*.h
	mkdir -p bin/macOS
	g++ -std=c++11 -Wall -Wno-deprecated-declarations -Wno-unused-function -g \
		-DWITH_MINIAUDIO -DMA_NO_RUNTIME_LINKING \
		-I ./include/ \
		-I ./soloud/include/ \
		-o ./bin/macOS/main \
		$(SRC) \
		-framework OpenGL -framework CoreFoundation -framework CoreAudio -framework AudioToolbox \
		-L/usr/local/lib -L/opt/homebrew/lib -L$(GLFW_LIB) \
		-lglfw -lm -lpthread

.PHONY: clean run
clean:
	rm -f bin/macOS/main

run: ./bin/macOS/main
	cd bin/macOS && ./main

else

./bin/Linux/main: src/* include/* CMakeLists.txt CMakePresets.json
	cmake --preset default-config
	cmake --build --preset default-build

.PHONY: clean run
clean:
	rm -f bin/Linux/main

run: ./bin/Linux/main
	cd bin/Linux && ./main

endif
