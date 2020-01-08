#ifndef JOYSTICKSTATUS_H
#define JOYSTICKSTATUS_H

#define AXIS_MAX (32767)
#define AXIS_MIN (-32768)
#define AXIS_THRESH 1000
#define JOY_FACT ((AXIS_MAX-AXIS_MIN+1)/1024)
#define JOY_OFFSET (512)

#define AXIS_BASE 128
#define HAT_BASE 192

#define PN(n) ((n+1)/2) //Converts -1/1 to 0/1
#define AXIS(n,d) (AXIS_BASE+n*2+PN(d))
#define HAT(x,y) (HAT_BASE+(y+1)*3+(x+1))

#define AXIS_NUM(n) ((n-AXIS_BASE)/2)
#define AXIS_DIR(n) (((n-AXIS_BASE) - (AXIS_NUM(n)*2))*2-1)

#define HAT_Y(n) ((n-HAT_BASE)/3-1)
#define HAT_X(n) ((n-HAT_BASE)-(HAT_Y(n)+1)*3-1)

#define NUM_BUTTONS 16
#define NUM_AXES 4

enum controller_type_t {
    NES = 0,
    SNES,
    N64
};

#define NAME_LEN 32
struct ButtonMapping {
  uint8_t version = 1;
  char name[NAME_LEN] = "";

  uint8_t NES_map[8] = {0};
  uint8_t SNES_map[12] = {0};
  uint8_t N64_map[16] = {0};

  uint8_t get_btn(controller_type_t type, uint8_t byte, uint8_t bit);
};

const signed short int hat_map[3][3] = {
  {315,0,45},
  {270,-1,90},
  {225,180,135}
};

class JoystickStatus {
public:
    signed short int axis[4];
    signed short int hat;
    uint8_t buttonset[2];
    controller_type_t controller_type;

    void translate_buttons(JoystickStatus *dest, ButtonMapping& button_map, controller_type_t type);
    void copyFrom(JoystickStatus *source);
    void clear();
    bool button_pressed();
    bool axis_pressed();
    bool hat_pressed();
    bool input_pressed();
};

#endif /* JOYSTICKSTATUS_H */
