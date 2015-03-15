#ifndef JOYSTICKSTATUS_H
#define JOYSTICKSTATUS_H

#define AXIS_MAX (32767)
#define AXIS_MIN (-32768)
#define AXIS_THRESH 1000
#define JOY_FACT ((AXIS_MAX-AXIS_MIN+1)/1024)
#define JOY_OFFSET (512)

#define PN(n) ((n+1)/2) //Converts -1/1 to 0/1
#define AXIS(n,d) (129+n*2+PN(d))
#define HAT(x,y) (192+(y+1)*3+(x+1))

#define NUM_BUTTONS 16

enum controller_type_t {
    NES = 0,
    SNES,
    N64
};

class JoystickStatus {
public:
    signed short int axis[3];
    signed short int hat;
    uint8_t buttonset[2];
    controller_type_t controller_type;

    void translate_buttons(JoystickStatus *dest, uint8_t *button_map);
    void copyFrom(JoystickStatus *source);
    void clear();
    bool button_pressed();
    bool axis_pressed();
    bool hat_pressed();
    bool input_pressed();
};

#endif /* JOYSTICKSTATUS_H */