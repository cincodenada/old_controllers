import processing.serial.*;

Serial N64Connection;
String buttons;
Robot VKey;
PImage bg;
String[] pressed;
boolean a,b,z,s,cu,cd,cl,cr,dd,du,dr,dl,ll,rr,u,d,l,r;
void setup() 
{
  pressed = split("a", ' ');;
  size(540,300);
  frameRate(1);
  String portName = Serial.list()[1];
  N64Connection = new Serial(this, portName, 115200);
  try
  {
    VKey = new Robot();
  }
  catch(AWTException a){}
  N64Connection.bufferUntil('\n');
  buttons = "Hell0 m0t0 ";
  bg = loadImage("N64 Controller.jpg");
}

void draw()
{
  background(bg);
  fill(255, 255, 0);
}

void serialEvent(Serial N64Connection)
{
  // bits: A, B, Z, Start, Dup, Ddown, Dleft, Dright
  // bits: 0, 0, L, R, Cup, Cdown, Cleft, Cright
  buttons = N64Connection.readString();
  pressed = split(buttons, ' ');
  //print(buttons);
  if(pressed.length >= 3)
  {
  if(!u && Integer.valueOf(pressed[2]) > 20)
  {
    VKey.keyPress(KeyEvent.VK_UP);
    u = !u;
  } else if (u && Integer.valueOf(pressed[2]) < 20) {
    VKey.keyRelease(KeyEvent.VK_UP);
    u = !u;
  }
  if(!d && Integer.valueOf(pressed[2]) < -20)
  {
    VKey.keyPress(KeyEvent.VK_DOWN);
    d = !d;
  } else if (d && Integer.valueOf(pressed[2]) > -20) {
    VKey.keyRelease(KeyEvent.VK_DOWN);
    d = !d;
  }
  if(!r && Integer.valueOf(pressed[1]) > 20)
  {
    VKey.keyPress(KeyEvent.VK_RIGHT);
    r = !r;
  } else if (r && Integer.valueOf(pressed[1]) < 20) {
    VKey.keyRelease(KeyEvent.VK_RIGHT);
    r = !r;
  }
  if(!l &&Integer.valueOf(pressed[1]) < -20)
  {
    VKey.keyPress(KeyEvent.VK_LEFT);
    l = !l;
  } else if(l && Integer.valueOf(pressed[1]) > -20) {
    VKey.keyRelease(KeyEvent.VK_LEFT);
    l = !l;
  }
  for (int i = 0; i <= 15; i++)
  {
    if (pressed[0].charAt(i) == ('4'))
    {
      switch(i)
      {
      case 0:
      if(!a)
      {
        VKey.keyPress(KeyEvent.VK_A);// A
        a = !a;
      }
        break;
      case 1:
      if(!b)
      {
        VKey.keyPress(KeyEvent.VK_B);// B
        b = !b;
      }
        break;
      case 2:
      if(!z)
      {
        VKey.keyPress(KeyEvent.VK_Z);// Z
        z = !z;
      }
        break;
      case 3:
        if(!s) 
        {
          VKey.keyPress(KeyEvent.VK_S);// Start
          s = !s;
        }
        break;
      case 4:
        if(!du) 
        {
          VKey.keyPress(KeyEvent.VK_NUMPAD8);// DUp
          du = !du;
        }
        break;
      case 5:
        if(!dd) 
        {
          VKey.keyPress(KeyEvent.VK_NUMPAD2);// DDown
          dd = !dd;
        }
        break;
      case 6:
        if(!dl) 
        {
          VKey.keyPress(KeyEvent.VK_NUMPAD4);// DLeft
          dl = !dl;
        }
        break;
      case 7:
        if(!dr) 
        {
          VKey.keyPress(KeyEvent.VK_NUMPAD6);// DRight
          dr = !dr;
        }
        break;
      case 10:
        if(!ll) 
        {
          VKey.keyPress(KeyEvent.VK_T);// L
          ll = !ll;
        }
        break;
      case 11:
        if(!rr) 
        {
          VKey.keyPress(KeyEvent.VK_R);// R
          rr = !rr;
        }
        break;  
      case 12:
        if(!cu) 
        {
          VKey.keyPress(KeyEvent.VK_I);// CUp
          cu = !cu;
        }
        break;
      case 13:
        if(!cd) 
        {
          VKey.keyPress(KeyEvent.VK_K);// CDown
          cd = !cd;
        }
        break;
      case 14:
        if(!cl) 
        {
          VKey.keyPress(KeyEvent.VK_J);// CLeft
          cl = !cl;
        }
        break;
      case 15:
        if(!cr) 
        {
          VKey.keyPress(KeyEvent.VK_L);// CRight
          cr = !cr;
        }
        break;      
    }}
    if (pressed[0].charAt(i) == ('0'))
    {
      switch(i)
      {
      case 0:
      if(a)
      {
        VKey.keyRelease(KeyEvent.VK_A);// A
        a = !a;
      }
        break;
      case 1:
      if(b)
      {
        VKey.keyRelease(KeyEvent.VK_B);// B
        b = !b;
      }
        break;
      case 2:
      if(z)
      {
        VKey.keyRelease(KeyEvent.VK_Z);// Z
        z = !z;
      }
        break;
      case 3:
        if(s) 
        {
          VKey.keyRelease(KeyEvent.VK_S);// Start
          s = !s;
        }
        break;
      case 4:
        if(du) 
        {
          VKey.keyRelease(KeyEvent.VK_NUMPAD8);// DUp
          du = !du;
        }
        break;
      case 5:
        if(dd) 
        {
          VKey.keyRelease(KeyEvent.VK_NUMPAD2);// DDown
          dd = !dd;
        }
        break;
      case 6:
        if(dl) 
        {
          VKey.keyRelease(KeyEvent.VK_NUMPAD4);// DLeft
          dl = !dl;
        }
        break;
      case 7:
        if(dr) 
        {
          VKey.keyRelease(KeyEvent.VK_NUMPAD6);// DRight
          dr = !dr;
        }
        break;
      case 10:
        if(ll) 
        {
          VKey.keyRelease(KeyEvent.VK_T);// L
          ll = !ll;
        }
        break;
      case 11:
        if(rr) 
        {
          VKey.keyRelease(KeyEvent.VK_R);// R
          rr = !rr;
        }
        break;  
      case 12:
        if(cu) 
        {
          VKey.keyRelease(KeyEvent.VK_I);// CUp
          cu = !cu;
        }
        break;
      case 13:
        if(cd) 
        {
          VKey.keyRelease(KeyEvent.VK_K);// CDown
          cd = !cd;
        }
        break;
      case 14:
        if(cl) 
        {
          VKey.keyRelease(KeyEvent.VK_J);// CLeft
          cl = !cl;
        }
        break;
      case 15:
        if(cr) 
        {
          VKey.keyRelease(KeyEvent.VK_L);// CRight
          cr = !cr;
        }
        break;  
      }}}}
}
