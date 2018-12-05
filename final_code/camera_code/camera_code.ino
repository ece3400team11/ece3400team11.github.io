#include <Wire.h>
#define OV7670_I2C_ADDRESS 0x21
#define COM7 0x12
#define COM3 0x0C
#define CLKRC 0x11
#define COM17 0x42
#define MVFP 0x1E
#define COM15 0x40
#define COM9 0x14
#define WB_B 0x01
#define WB_R 0x02
#define WB_G 0x6A
#define AEC  0x07
#define G_LSB 0x00
#define G_MSB 0x03
#define COM8 0x13
#define AEC_M 0x10

///////// Main Program //////////////
void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.println("Starting");
  // TODO: READ KEY REGISTERS

  // result = OV7670_write_register(0x0C, 0b00000000);

  String result = OV7670_write_register(COM7, 0x80);
  Serial.println(result);
   
  set_color_matrix();

  //delay(200);

  // 0x0E for color bar
  result = OV7670_write_register(COM7, 0x0C);
  Serial.println(result);

  result = OV7670_write_register(COM3, 0x08);
  Serial.println(result);

  result = OV7670_write_register(CLKRC, 0xC0);
  Serial.println(result);

  //result = OV7670_write_register(COM17, 0x08);
  //Serial.println(result);

  result = OV7670_write_register(COM15, 0xD0);
  Serial.println(result);

  result = OV7670_write_register(COM9, 0x6A);
  Serial.println(result);

  result = OV7670_write_register(MVFP, 0x30);
  Serial.println(result);

  //result = OV7670_write_register(COM8, 0x85);

//  delay(5000);
  
  result = OV7670_write_register(COM8, 0x88);
  Serial.println(result);

  result = OV7670_write_register(WB_B, 0x80);
  Serial.println(result);

  result = OV7670_write_register(WB_R, 0x40);
  Serial.println(result);

  result = OV7670_write_register(WB_G, 0x4E);
  Serial.println(result);

  result = OV7670_write_register(AEC, 0x40);
  Serial.println(result);

  result = OV7670_write_register(G_LSB, 0x0F);
  Serial.println(result);

  result = OV7670_write_register(G_MSB, 0x00);
  Serial.println(result);

  result = OV7670_write_register(AEC_M, 0x24);
  Serial.println(result);

//  result = OV7670_write_register(0x41, 0x18);
//  Serial.println(result);

  // https://github.com/dalmirdasilva/ArduinoCamera/blob/master/CameraAL422B/datasheet/OV7670%20Implementation%20Guide%20(V1.0).pdf
  result = OV7670_write_register(0x4C, 0xFF);
  Serial.println(result);

  result = OV7670_write_register(0x55, 0x60);
  Serial.println(result);
////
  result = OV7670_write_register(0x56, 0x80);
  Serial.println(result);

  delay(100);
  
  // TODO: WRITE KEY REGISTERS
  read_key_registers();

//  // put your setup code here, to run once:
//  pinMode(4, INPUT);
//  Serial.begin(9600);
//  Serial.println("Starting img detection");
}

void loop(){
   read_result();
 }


///////// Function Definition //////////////
void read_key_registers(){
  /*TODO: DEFINE THIS FUNCTION*/
  byte data = read_register_value(COM3);
  Serial.print("0x0C = ");
  Serial.println(data);

  data = read_register_value(CLKRC);
  Serial.print("0x11 = ");
  Serial.println(data);

  data = read_register_value(COM7);
  Serial.print("0x12 = ");
  Serial.println(data);

  data = read_register_value(COM17);
  Serial.print("0x42 = ");
  Serial.println(data);  

  data = read_register_value(MVFP);
  Serial.print("0x1E = ");
  Serial.println(data);

  delay(7000);
  
  data = read_register_value(0x01);
  Serial.print("0x01 = ");
  Serial.println(data);

  data = read_register_value(0x02);
  Serial.print("0x02 = ");
  Serial.println(data);

  data = read_register_value(0x6A);
  Serial.print("0x6A = ");
  Serial.println(data);

  data = read_register_value(0x07);
  Serial.print("0x07 = ");
  Serial.println(data);

  data = read_register_value(0x00);
  Serial.print("0x00 = ");
  Serial.println(data);

  data = read_register_value(0x03);
  Serial.print("0x03 = ");
  Serial.println(data);

  data = read_register_value(0x10);
  Serial.print("0x10 = ");
  Serial.println(data);
}

byte read_register_value(int register_address){
  byte data = 0;
  Wire.beginTransmission(OV7670_I2C_ADDRESS);
  Wire.write(register_address);
  Wire.endTransmission();
  Wire.requestFrom(OV7670_I2C_ADDRESS,1);
  while(Wire.available()<1);
  data = Wire.read();
  return data;
}

String OV7670_write(int start, const byte *pData, int size){
    int n,error;
    Wire.beginTransmission(OV7670_I2C_ADDRESS);
    n = Wire.write(start);
    if(n != 1){
      return "I2C ERROR WRITING START ADDRESS";   
    }
    n = Wire.write(pData, size);
    if(n != size){
      return "I2C ERROR WRITING DATA";
    }
    error = Wire.endTransmission(true);
    if(error != 0){
      return "error"+String(error);
    }
    return "no errors :)";
 }

String OV7670_write_register(int reg_address, byte data){
  return OV7670_write(reg_address, &data, 1);
 }

void set_color_matrix(){
    OV7670_write_register(0x4f, 0x80);
    OV7670_write_register(0x50, 0x80);
    OV7670_write_register(0x51, 0x00);
    OV7670_write_register(0x52, 0x22);
    OV7670_write_register(0x53, 0x5e);
    OV7670_write_register(0x54, 0x80);
    OV7670_write_register(0x56, 0x40);
    OV7670_write_register(0x58, 0x9e);
    OV7670_write_register(0x59, 0x88);
    OV7670_write_register(0x5a, 0x88);
    OV7670_write_register(0x5b, 0x44);
    OV7670_write_register(0x5c, 0x67);
    OV7670_write_register(0x5d, 0x49);
    OV7670_write_register(0x5e, 0x0e);
    OV7670_write_register(0x69, 0x00);
    OV7670_write_register(0x6a, 0x40);
    OV7670_write_register(0x6b, 0x0a);
    OV7670_write_register(0x6c, 0x0a);
    OV7670_write_register(0x6d, 0x55);
    OV7670_write_register(0x6e, 0x11);
    OV7670_write_register(0x6f, 0x9f);
    OV7670_write_register(0xb0, 0x84);
}

void read_result(){
  int w0 = analogRead(A1) > 500;
  int w1 = analogRead(A2) > 500;
  int w2 = analogRead(A3) > 500;
  
  if (!w2 && !w1 && w0) {
    Serial.println("Blue triangle");
  } else if (!w2 && w1 && !w0) {
    Serial.println("Blue square");
  } else if (!w2 && w1 && w0) {
    Serial.println("Blue diamond");
  } else if (w2 && !w1 && w0) {
    Serial.println("Red triangle");
  } else if (w2 && w1 && !w0) {
    Serial.println("Red square");
  } else if (w2 && w1 && w0) {
    Serial.println("Red diamond");
  } else if (!w1 && !w0) {
    Serial.println("Nothing");
  }
}
