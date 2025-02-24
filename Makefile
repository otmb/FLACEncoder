
NAME = emscripten-flac
DATE := $(shell date +%Y-%m-%d)
DIR := $(shell pwd)

docker-build:; podman build . \
	-t otmb/$(NAME) \
	-t otmb/$(NAME):$(DATE) \
	-f ./Dockerfile

wasm-build:; podman run --rm \
	-v $(DIR):/src \
	otmb/emscripten-flac \
	bash ./build.sh
