
#include <Wire.h>

#define    LIDARLite_ADDRESS   0x62          // Default I2C Address of LIDAR-Lite.
#define    RegisterMeasure     0x00          // Register to write to initiate ranging.
#define    MeasureValue        0x04          // Value to initiate ranging.
#define    RegisterHighLowB    0x8f          // Register to get both High and Low bytes in 1 call.

#define    RoboRIO_ADDRESS     0

#define    MIN_DISTANCE        0
#define    MAX_DISTANCE        4000
#define    AVERAGING_THRESHOLD 3


int lastDistance = 0;

void setup() {
  Wire.begin();
  Serial.begin(115200);
}

void loop() {
  
  lidar();
  
}

void lidar() {

  requestRanging();
  
  if(requestOutput()) 
  {
    lastDistance = getDistance();
    Serial.println(lastDistance);
    
    Wire.beginTransmission((int)RoboRIO_ADDRESS);
    Wire.write(lastDistance);
    Wire.endTransmission();
    
  }
}

void requestRanging() {
  Wire.beginTransmission((int)LIDARLite_ADDRESS); // transmit to LIDAR-Lite
  
  Wire.write((int)RegisterMeasure); // Register to write to (0x00)  
  Wire.write((int)MeasureValue); // Value written to register to start ranging
  
  Wire.endTransmission(); // stop transmitting
  delay(20); // Wait 20ms for transmit
}

bool requestOutput() {
  Wire.beginTransmission((int)LIDARLite_ADDRESS); // transmit to LIDAR-Lite
  
  Wire.write((int)RegisterHighLowB); // sets register pointer to (0x8f)
  
  Wire.endTransmission(); // stop transmitting
  delay(20); // Wait 20ms for transmit
  Wire.requestFrom((int)LIDARLite_ADDRESS, 2); // request 2 bytes from LIDAR-Lite
  return (2 <= Wire.available()); // if two bytes were received
}

int getDistance() {
  int centimeterDistance;
  
  centimeterDistance = Wire.read(); // receive high byte (overwrites previous reading)
  centimeterDistance = centimeterDistance << 8; // shift high byte to be high 8 bits
  centimeterDistance |= Wire.read(); // receive low byte as lower 8 bits

  if(centimeterDistance < MIN_DISTANCE) centimeterDistance = MIN_DISTANCE;
  if(centimeterDistance > MAX_DISTANCE) centimeterDistance = MAX_DISTANCE;
  if(abs(centimeterDistance - lastDistance) < AVERAGING_THRESHOLD) centimeterDistance = (centimeterDistance + lastDistance) / 2;
  
  return centimeterDistance;
}

