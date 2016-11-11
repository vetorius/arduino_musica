#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>

// display definitions

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// mp3 player definitions

#define BUSY 8

// encoder definitions

#define ENA 7
#define ENB 5
#define SW 6

#define MAXPOS 10
int POS = 1;
int PUL;
int LPUL = 0;

// program definitions

int state = 0;
/*
 * state = 1 stop
 * state = 2 play
 * state = 3 alarm
 * state = 4 timer
 * state = otherwise error
 */

unsigned long tiempo;

int mp3_busy = 1;


void setup() {
  
  Serial1.begin (9600);

  mp3_set_serial (Serial1);      //set Serial for DFPlayer-mini mp3 module 
  delay (1);                     // delay 10ms to set volume
  mp3_set_volume (10);
  pinMode(BUSY, INPUT);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);
  display.clearDisplay();
  
  // Entradas digitales para el encoder rotativo
  pinMode(ENA, INPUT); // ENA
  pinMode(ENB, INPUT); // ENB
  pinMode(SW, INPUT); // Pulsador
  attachInterrupt(4, encoder_int, RISING);
  change_state(1);
}

void loop() {
  mp3_busy = digitalRead(BUSY);
  encoder();
  pantalla();

  if ((state == 4) and (millis() > tiempo)) {
    change_state(1);
  }
  if ((state == 2) and (mp3_busy == 1)) {
    change_state(1);
  }
  if ((state == 3) and (mp3_busy == 1)) {
    change_state(3);
  }
  delay(10);
}

void pantalla(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  switch (state) {
    case 1:
      display.println("STOP");
      break;
    case 2:
      display.println("PLAY");
      break;
    case 3:
      display.println("ALARM ON");
      break;
    case 4:
      display.println("TIMER " + tiempo_restante(tiempo-millis()));
      break;
   default:
      display.println("ERROR!");
    break;
  }
  if (POS>0){
    display.println("SONG: " + String(POS));
  } else {
    display.println("--> ALARM");
  }
//  display.println(String(millis()));
  display.println(String(mp3_busy));
  display.display();
}

void encoder() {
  PUL = !digitalRead(SW);
  if ((PUL != LPUL) and (PUL == 1)) {
    if (state == 1) {
      if (POS>0){
        change_state(2);
      } else {
        change_state(3);
      }
    } else {
      change_state(1);
    }
  }
  LPUL = PUL;
}

void encoder_int() {
  if (digitalRead(ENB) == digitalRead(ENA)) {
    if (POS < MAXPOS) {
      POS++; //if encoder channels are the same, direction is CW
    }
  } else {
    if (POS > 0) {
      POS--; //if they are not the same, direction is CCW
    }
  }
}

void change_state(int new_state){
  switch (new_state) {
    case 1:
      mp3_stop();
      break;
    case 2:
      mp3_play(POS+1);
      espera_busy();      
      break;
    case 3:
      mp3_play(POS+1);
      espera_busy(); 
      break;
    case 4:
      tiempo = millis() + 240000;
      mp3_play(POS+1);      
      espera_busy(); 
      break;
  }
  state = new_state;
  delay(10);
}

void espera_busy(){
  while (mp3_busy == 1){
    mp3_busy = digitalRead(BUSY);
  }
}

String tiempo_restante(unsigned long restante){
  
  unsigned int seg, minutos, segundos;
  
  seg = restante / 1000;
  minutos = seg / 60;
  segundos = seg % 60;
  if (segundos > 9){
    return String(minutos)+":"+String(segundos);
  } else {
    return String(minutos)+":0"+String(segundos);
  }
  
}

