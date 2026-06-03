OS := $(shell uname)

ifeq ($(OS), Darwin)

GLFW_LIB := $(shell brew --prefix glfw 2>/dev/null)/lib

./bin/macOS/main: src/*.cpp src/*.c include/*.h
	mkdir -p bin/macOS
	g++ -std=c++11 -Wall -Wno-deprecated-declarations -Wno-unused-function -g \
		-I ./include/ \
		-o ./bin/macOS/main \
		src/main.cpp src/correcao.cpp src/glad.c src/textrendering.cpp src/tiny_obj_loader.cpp src/stb_image.cpp \
		-framework OpenGL -L/usr/local/lib -L/opt/homebrew/lib -L$(GLFW_LIB) \
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
