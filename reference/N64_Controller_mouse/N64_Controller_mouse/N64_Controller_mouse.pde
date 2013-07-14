import processing.serial.*;

Serial NESjoy;
String buttons;
Robot VKey;
PImage bg;
String[] pressed;
boolean a,b,z,s,cu,cd,cl,cr,dd,du,dr,dl,ll,rr,u,d,l,r;
int mx = 0, my = 0;
int sx, sy;
boolean mouse = false;
void setup() 
{
  pressed = split("a", ' ');;
  size(434,180);
  frameRate(1);
  String portName = Serial.list()[1];
  NESjoy = new Serial(this, portName, 115200);
  try
  {
    VKey = new Robot();
  }
  catch(AWTException a){}
  NESjoy.bufferUntil('\n');
  buttons = "Hell0 m0t0 ";
  bg = loadImage("NES-Controller.jpg");
}

void draw()
{
  background(bg);
  fill(255, 255, 0);
  
  // bits: A, B, Z, Start, Dup, Ddown, Dleft, Dright
  // bits: 0, 0, L, R, Cup, Cdown, Cleft, Cright
}

void serialEvent(Serial NESjoy)
{
  buttons = NESjoy.readString();
  pressed = split(buttons, ' ');
  //print(buttons);
  if(pressed.length >= 3)
  {
  if(mouse){//allow mouse mov't to be turned on and off
    if(abs(Integer.valueOf(pressed[2])) > 5) my -= Integer.valueOf(pressed[2]);
    if(abs(Integer.valueOf(pressed[1])) > 5) mx += Integer.valueOf(pressed[1]);
    VKey.mouseMove(mx,my);

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
        VKey.mousePress(InputEvent.BUTTON1_MASK);// A
        a = !a;
      }
        break;
      case 1:
      if(!b)
      {
        VKey.mousePress(InputEvent.BUTTON3_MASK);// B
        b = !b;
      }
        break;
      case 2:
      if(!z)
      {
        VKey.mousePress(InputEvent.BUTTON2_MASK);// Z
        z = !z;
      }
        break;
      case 3:
        if(!s) 
        {
          //VKey.keyPress(KeyEvent.VK_S);// Start
          s = !s;
          mx = java.awt.MouseInfo.getPointerInfo().getLocation().x;
          my = java.awt.MouseInfo.getPointerInfo().getLocation().y;
          mouse = !mouse;//have the start button enable/disable the mouse
        }
        break;
      case 4:
        if(!du) 
        {
          VKey.keyPress(KeyEvent.VK_UP);// DUp
          du = !du;
        }
        break;
      case 5:
        if(!dd) 
        {
          VKey.keyPress(KeyEvent.VK_DOWN);// DDown
          dd = !dd;
        }
        break;
      case 6:
        if(!dl) 
        {
          VKey.keyPress(KeyEvent.VK_LEFT);// DLeft
          dl = !dl;
        }
        break;
      case 7:
        if(!dr) 
        {
          VKey.keyPress(KeyEvent.VK_RIGHT);// DRight
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
        VKey.mouseRelease(InputEvent.BUTTON1_MASK);// A
        a = !a;
      }
        break;
      case 1:
      if(b)
      {
        VKey.mouseRelease(InputEvent.BUTTON3_MASK);// B
        b = !b;
      }
        break;
      case 2:
      if(z)
      {
        VKey.mouseRelease(InputEvent.BUTTON3_MASK);// Z
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
          VKey.keyRelease(KeyEvent.VK_UP);// DUp
          du = !du;
        }
        break;
      case 5:
        if(dd) 
        {
          VKey.keyRelease(KeyEvent.VK_DOWN);// DDown
          dd = !dd;
        }
        break;
      case 6:
        if(dl) 
        {
          VKey.keyRelease(KeyEvent.VK_LEFT);// DLeft
          dl = !dl;
        }
        break;
      case 7:
        if(dr) 
        {
          VKey.keyRelease(KeyEvent.VK_RIGHT);// DRight
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
