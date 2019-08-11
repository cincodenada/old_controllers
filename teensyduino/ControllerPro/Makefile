default: build

.PHONY: upload

build: *.cpp *.h *.ino
	./build.sh --verify

upload:
	./build.sh --upload
