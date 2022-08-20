.PHONY: build run

build:
	gcc -g -o eomsort.so -shared eomsort.c $(shell pkg-config --libs --cflags glib-2.0 libpeas-1.0 eom)
