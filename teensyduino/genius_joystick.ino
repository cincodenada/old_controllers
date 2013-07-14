/* Emilio Moretti

Serial to USB converter.

Makes my Genius 2000 F-16 work in new computers.

Copyright 2012 Emilio Moretti <emilio.morettiATgmailDOTcom>
This program is distributed under the terms of the GNU Lesser General Public License.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU  Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
int scale_analog_read(int *v_max,int *v_min,int *v_center,int *v_deadzone,int value){
  if (value < *v_min){
    *v_min = value;
  }
  else if(value > *v_max){
    *v_max = value;
  }
  
  if ((value >= *v_center - *v_deadzone) & (value <= *v_center + *v_deadzone))
  {
    value = *v_center;
  }

  float delta = float(*v_max - *v_min)/1024.0;

  return (value-*v_min)/delta;
}

void correct_deadzone(int *v_center,int *v_deadzone, int value){
    if (value > *v_center + *v_deadzone){
      int temp_min_v = *v_center - *v_deadzone;
      int temp_max_v = value;
      *v_center = (temp_max_v + temp_min_v)/2;
      int temp_deadzone_1 = *v_center - temp_min_v;
      int temp_deadzone_2 = temp_max_v - *v_center;
      if (temp_deadzone_1 > temp_deadzone_2){
        *v_deadzone = temp_deadzone_1;
      }
      
      else{
        *v_deadzone = temp_deadzone_2;
      }
    }
    else if ( value < *v_center - *v_deadzone){
      int temp_min_v = value;
      int temp_max_v = *v_center + *v_deadzone;;
      *v_center = (temp_max_v + temp_min_v)/2;
      int temp_deadzone_1 = *v_center - temp_min_v;
      int temp_deadzone_2 = temp_max_v - *v_center;
      if (temp_deadzone_1 > temp_deadzone_2){
        *v_deadzone = temp_deadzone_1;
      }
      else{
        *v_deadzone = temp_deadzone_2;
      }
    } 
}

void setup() {
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  //Serial.begin(9600); // USB is always 12 Mbit/sec
  
  x_center = analogRead(0);
  y_center = analogRead(1);
  z_center = analogRead(2);
  x_min = x_center;
  x_max = x_center;
  y_min = y_center;
  y_max = y_center;
  z_min = z_center;
  z_max = z_center;
    
  int counter = 200; 
  while (counter > 0 )
  {
    delay(50);
    int x = analogRead(0);
    int y = analogRead(1);
    int z = analogRead(2);

    //fix X deadzone
    correct_deadzone(&x_center,&x_deadzone, x);

    //fix Y deadzone
    correct_deadzone(&y_center,&y_deadzone, y);

    //fix Z deadzone
    correct_deadzone(&z_center,&z_deadzone, z);
    counter--;
    
  }

  digitalWrite(ledPin,LOW);
  
}

void loop() {
  /*
  DEBUG CODE
  int temp = analogRead(0);
  int x = scale_analog_read_x(temp);
  Serial.println("REAL x");
  Serial.println(temp);
  Serial.println("x_center:");
  Serial.println(x_center);
  
  Serial.println("x_deadzone:");
  Serial.println(x_deadzone);
  Serial.println("x scaled:");
  Serial.println(x);
  delay(1000);
  */
  int x = scale_analog_read(&x_max,&x_min,&x_center,&x_deadzone,analogRead(0));
  int y = scale_analog_read(&y_max,&y_min,&y_center,&y_deadzone,analogRead(1));
  int z = scale_analog_read(&z_max,&z_min,&z_center,&z_deadzone,analogRead(2));

  
  // read analog inputs and set X-Y position
  Joystick.X(x);
  Joystick.Y(y);
  Joystick.Z(z);

  // read the digital inputs and set the buttons
  Joystick.button(1, !digitalRead(0));
  Joystick.button(2, !digitalRead(1));
  Joystick.button(3, !digitalRead(2));
  Joystick.button(4, !digitalRead(3));

  // a brief delay, so this runs 20 times per second
  delay(50);
}
