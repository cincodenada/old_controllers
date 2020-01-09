#!/bin/bash -x

ARDUINO=/opt/arduino/arduino
# Teensyduino not supported here, it seems :(
#ARDUINO=arduino-cli

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

if [[ "$ARDUINO" == *"arduino-cli" ]]; then
  rm -rf build
  mkdir -p build
  $ARDUINO compile -b teensy:avr:$BOARD:usb=$USB --build-path build $@ .
else
  $ARDUINO --board teensy:avr:$BOARD:usb=$USB --preserve-temp-files $@ ControllerPro.ino
fi
