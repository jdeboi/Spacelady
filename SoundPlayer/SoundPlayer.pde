// Sound 
import ddf.minim.*;
Minim minim;
AudioSample[] sounds;
Button[] soundButtons;

// Serial
import processing.serial.*;
Serial myPort;  
int useSerial = -1;

void setup() {
  size(600, 500, P3D);
  loadSounds();
  loadButtons();
}

void draw() {
  background(255);
  if ( useSerial < 0){
    getSerialPort();
    //println(useSerial);
    return;
  }
  getData();
  drawButtons();
}

void loadButtons() {
  soundButtons = new Button[12];
  int x = 15;
  for(int i = 0; i<12; i++) {
    soundButtons[i] = new Button(i, x, 100, 40, 40);
    x+=48;
  }
} 

void drawButtons() {
  for(Button button:soundButtons) {
    soundButtons[button.n].drawButton();
  }
}

void getData() {
  if (myPort.available() > 0) { 
    int val = myPort.read(); 
    if(val < 12) { 
      //println(val);
      sounds[val].trigger(); 
      soundButtons[val].notePressed();
    }  
  }
}

void getSerialPort(){
  String[] ports = Serial.list();
  int i;
  int x = 20;
  int y0 = 50;
  fill(0);
  text("Select serial port:", x, 30);
  Button[] setupButtons = new Button[ports.length+1];
  for(i = 0; i < ports.length; i++){
    setupButtons[i] = new Button(i, x, 30*i+y0, 20, 20);
    fill(0);
    text(ports[i], x+30, 30*i+y0+15);
  }
  setupButtons[i] = new Button(i, x, 30*i+y0, 20, 20);
  fill(0);
  text("No serial", x+30, 30*i+y0+15);
  delay(10);
  for(Button button:setupButtons) {
    if(button.contains()){
      button.state = true;
      button.highlight = true;
      if(mousePressed) {
        if( button.n == ports.length ){
          useSerial = 0;
          return;
        } else {
          useSerial = button.n;
          myPort = new Serial(this, ports[button.n], 9600);
          return;
        }
      }
    }
    else{
      button.state = false;
      button.highlight = false;
    }
    button.drawButton();
  }
  return;
}

void loadSounds() {
  minim = new Minim(this);
  sounds = new AudioSample[12];
  for(int i=0; i<12; i++) {
    println(i);
    String f = "audio/" + i + ".wav";
    sounds[i] = minim.loadSample(f, 512);
  }
}
