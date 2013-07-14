if [ $1 ]; then
    BUILD_DIR=$1
fi

if [ $2 ]; then
    FILE=$2
fi

/opt/arduino/hardware/tools/avr/bin/avr-objdump -S $BUILD_DIR/$FILE.cpp.elf > $FILE.cpp.asm
