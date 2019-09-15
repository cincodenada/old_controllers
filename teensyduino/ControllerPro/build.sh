#!/bin/bash -x

BOARD="teensy31"
USB="serialmultijoy"
while true; do
    case $1 in
        teensy*)
            BOARD=$1
            shift
            ;;
        *multijoy)
            USB=$1
            shift
            ;;
        *)
            break;
            ;;
    esac
done

# Alias this for friendliness
if [ "$BOARD"=="teensy32" ]; then
    BOARD="teensy31"
fi

echo "Using board $BOARD";
echo "Using mode $USB";

arduino --board teensy:avr:$BOARD:usb=$USB --preserve-temp-files $@ N64_Arduino.ino