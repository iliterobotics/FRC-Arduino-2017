#include <Adafruit_NeoPixel.h>
#include <NeoPixelPainter.h>
#include <Wire.h>

#define NUM_PIXELS 30
#define ACTUAL_NUM_PIXELS 7
#define LED_PIN   8
#define DEVICE_ID  1

#define LED_SPEED   900
#define FADE_SPEED  700

Adafruit_NeoPixel topStrip = Adafruit_NeoPixel(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

NeoPixelPainterCanvas topCanvas = NeoPixelPainterCanvas(&topStrip); //create canvas, linked to the neopixels (must be created before the brush)
NeoPixelPainterBrush topBrush = NeoPixelPainterBrush(&topCanvas); //crete brush, linked to the canvas to paint to

String receivedCommand = "";
char term = ';';

String command = "";
int desiredR, desiredG, desiredB = 0;
long ledDelay = 0;

//blinkLED variables
long lastLedTime = 0;
bool on = true;

//pulse variables
int brightness = 255;
bool add = true;

//crazy variables
bool crazyOn = true;

void setup() {

  pinMode(LED_PIN, OUTPUT);

  Wire.begin(DEVICE_ID);
  Wire.onReceive(receiveEvent);

  Serial.begin(115200);
  Serial.println("LED Code");

  if (topCanvas.isvalid() == false) Serial.println("canvas allocation problem");
  else  Serial.println("canvas allocation ok");
  if (topBrush.isvalid() == false) Serial.println("brush allocation problem");
  else  Serial.println("brush allocation ok");

  topStrip.setBrightness(255);
}

void loop() {
  if(command.equals("default")) {
    defaultLED(&topStrip, &topCanvas, &topBrush);
  } else if(command.equals("solid")) {
    setStripColor(desiredR, desiredG, desiredB, &topStrip);
  }else if(command.equals("pulse")) {
    pulse(desiredR, desiredG, desiredB, &topStrip);
  }else if(command.equals("run")) {
    runLED(desiredR, desiredG, desiredB, &topStrip, &topCanvas, &topBrush, false);
  }else if(command.equals("blink")) {
    if(lastLedTime + ledDelay < millis()) blinkLED(desiredR, desiredG, desiredB, &topStrip);
  }else if(command.equals("crazy")) {
    crazy(&topStrip);
  } else if(command.equals("progress")) {
    
  } else {
    defaultLED(&topStrip, &topCanvas, &topBrush);
  }
   
}

void parseMessage(String message)
{
  Serial.println("Parsing");
  int termIndex = 0;

  termIndex = message.indexOf(term);
  
  termIndex = message.indexOf(term);
  String requestedCommand = message.substring(0, termIndex);
  message = message.substring(termIndex + 1, message.length());
  
  termIndex = message.indexOf(term);
  int requestedR = message.substring(0, termIndex).toInt();
  message = message.substring(termIndex + 1, message.length());
  
  termIndex = message.indexOf(term);
  int requestedG = message.substring(0, termIndex).toInt();
  message = message.substring(termIndex + 1, message.length());

  termIndex = message.indexOf(term);
  int requestedB = message.substring(0, termIndex).toInt();
  message = message.substring(termIndex + 1, message.length());
  
  termIndex = message.indexOf(term);
  int requestedDelay = message.substring(0, termIndex).toInt();
  message = message.substring(termIndex + 1, message.length());

  command = requestedCommand;
  desiredR = requestedR;
  desiredG = requestedG;
  desiredB = requestedB;
  ledDelay = requestedDelay;
  Serial.println("Command: " + command + " R: " + desiredR + " g: " + desiredG + " b: " + desiredB + " Delay: " + ledDelay);
}

void receiveEvent(int howMany) {
  topStrip.clear();
  crazyOn = true;
  receivedCommand = "";
  while(Wire.available() > 0)
    {
        char c = Wire.read();                 //Read a character from the wire
        receivedCommand += c;   
        Serial.print(c);
    }
    parseMessage(receivedCommand);
}

void setStripColor(int r, int g, int b, Adafruit_NeoPixel * strip) {
    for(uint16_t i = 0; i < strip->numPixels(); i++) {
        strip->setPixelColor(i, r, g, b);
    }
    strip->show();
}

void pulse(int r, int g, int b, Adafruit_NeoPixel * strip) {
  if(brightness >= 255 || brightness <= 0) add = !add;
  if(add) {
    brightness += 1;
  } else {
    brightness -= 1;
  }
  
  setStripColor(brightness * desiredR / 255, brightness * desiredG / 255, brightness * desiredB / 255, strip);
}

void runLED(int h, int s, int v, Adafruit_NeoPixel * strip, NeoPixelPainterCanvas * canvas, NeoPixelPainterBrush * brush, bool bounce) {
       HSV brushColor;
       brushColor.h = h; 
       brushColor.s = s; //full saturation
       brushColor.v = v; //medium brightness
 
       brush->setSpeed(LED_SPEED); 
       brush->setFadeSpeed(FADE_SPEED); 
       brush->setColor(brushColor); //update the color of the brush
       brush->setFadeout(true);
       brush->setFadeHueNear(false); //fade using the near path on the colorcircle
       brush->setBounce(bounce);
      
       strip->clear();

       brush->paint(); //apply the paint of the first brush to the canvas (and update the brush)
       canvas->transfer(); //transfer the canvas to the neopixels
       strip->show();

}

void blinkLED(int r, int g, int b, Adafruit_NeoPixel * strip) {
  lastLedTime = millis();
  if(on) {
    setStripColor(r, g, b, strip);
  } else {
    setStripColor(0, 0, 0, strip);
  }

  on = !on;
  
}

void crazy(Adafruit_NeoPixel * strip) {
  if(!crazyOn) return;
  crazyOn = false;
  for(int i = 0; i < strip->numPixels(); i ++) {
    int randNum = random(0, 3);
    if(randNum == 0) strip->setPixelColor(i, 255, 255, 255);
    if(randNum == 1) strip->setPixelColor(i, 0, 255, 0);
    if(randNum == 2) strip->setPixelColor(i, 255, 0, 255);
  }
  strip->show();
}

void progressBar(int r, int g, int b, int progress, Adafruit_NeoPixel * strip) {
  int bars = progress / (100 / ACTUAL_NUM_PIXELS);
  for(int i = 0; i <= bars; i++) {
    strip->setPixelColor(i, r, g, b);
  }
  strip->show();
}

void defaultLED(Adafruit_NeoPixel * strip, NeoPixelPainterCanvas * canvas, NeoPixelPainterBrush * brush) {
  strip->clear();
  runLED(desiredR, desiredG, desiredG, strip, canvas, brush, true);
}

