default: build

.PHONY: build upload

build: *.cpp *.h *.ino
	./build.sh --verify

upload:
	./build.sh --upload
