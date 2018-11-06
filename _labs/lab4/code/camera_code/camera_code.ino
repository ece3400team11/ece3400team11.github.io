#include <Wire.h>

// decimal 42
#define OV7670_I2C_ADDRESS 0x21
#define COM7 0x12
#define COM3 0x0C
#define CLKRC 0x11
#define SCALING_XSC 0x70
#define SCALING_YSC 0x71
#define COM17 0x42
#define MVFP 0x1E
#define COM8 0x13
#define COM15 0xC0
#define COM9 0x14
#define COM16 0x41

///////// Main Program //////////////
void setup() {
  Wire.begin();
  Serial.begin(9600);
  set_color_matrix();
  Serial.println("Starting");
  
  // TODO: READ KEY REGISTERS

  // result = OV7670_write_register(0x0C, 0b00000000);

  String result = OV7670_write_register(COM7, 0b10000000);
  Serial.println(result);

  delay(200);

  result = OV7670_write_register(COM7, 0b00001110);
  Serial.println(result);

//  result = OV7670_write_register(COM3, 0b00001000);
//  Serial.println(result);

  result = OV7670_write_register(CLKRC, 0b11000000);
  Serial.println(result);

  //result = OV7670_write_register(SCALING_XSC, 0b10000011);
  //Serial.println(result);

  //result = OV7670_write_register(SCALING_YSC, 0b00000011);
  //Serial.println(result);

  result = OV7670_write_register(COM17, 0b00001000);
  Serial.println(result);

  result = OV7670_write_register(COM15, 0b11010000);
  Serial.println(result);

//  result = OV7670_write_register(COM9, 0b01101010);
//  Serial.println(result);

//  result = OV7670_write_register(COM16, 0b00011000);
//  Serial.println(result);

//  result = OV7670_write_register(MVFP, 0b0010000);
//  Serial.println(result);
  
//  result = OV7670_write_register(COM8, 0b10000010);
//  Serial.println(result);
  
  delay(100);
  
  // TODO: WRITE KEY REGISTERS
  
  read_key_registers();
}

void loop(){
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

  data = read_register_value(SCALING_XSC);
  Serial.print("0x70 = ");
  Serial.println(data);

  data = read_register_value(SCALING_YSC);
  Serial.print("0x71 = ");
  Serial.println(data);

  data = read_register_value(COM17);
  Serial.print("0x42 = ");
  Serial.println(data);  

  data = read_register_value(MVFP);
  Serial.print("0x1E = ");
  Serial.println(data);

  data = read_register_value(COM8);
  Serial.print("0x13 = ");
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
