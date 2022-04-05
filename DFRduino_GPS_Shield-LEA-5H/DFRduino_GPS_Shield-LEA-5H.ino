// # Editor     : Lauren from DFRobot
// # Date       : 22.02.2012

// # Product name:


// # Product SKU : TEL0044
// # Version     : 1.0

// # Update the library and sketch to compatible with IDE V1.0 and earlier

// # Description:
// # The sketch for using the DFRduino GPS Shield-LEA-5H

// # Connection:
// #        Directly connect the shield to the Arduino controller
// #        If you'd like to drive the shield via UART interface, you may need to connect the jumpers on the board.
// #


#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#define WireSend(args) Wire.write(args)
#define WireRead(args) Wire.read(args)
#define printByte(args) Serial.write(args)
#define printlnByte(args)  Serial.write(args),Serial.println()
#else
#include "WProgram.h"
#define WireSend(args) Wire.send(args)
#define WireRead(args) Wire.receive(args)
#define printByte(args) Serial.print(args,BYTE)
#define printlnByte(args)  Serial.println(args,BYTE)
#endif

#include <Wire.h>

#define BUFFER_LENGTH 10//Define the buffer length

int GPSAddress = 0x42;//GPS I2C Address

double Datatransfer(char *data_buf, char num) //Data type converter：convert char type to float
{ //*data_buf:char data array ;num:float length
  double temp = 0.0;
  unsigned char i, j;

  if (data_buf[0] == '-') //The condition of the negative
  {
    i = 1;
    //The date in the array is converted to an integer and accumulative
    while (data_buf[i] != '.')
      temp = temp * 10 + (data_buf[i++] - 0x30);
    for (j = 0; j < num; j++)
      temp = temp * 10 + (data_buf[++i] - 0x30);
    //The date will converted integer transform into a floating point number
    for (j = 0; j < num; j++)
      temp = temp / 10;
    //Converted to a negative number
    temp = 0 - temp;
  }
  else//Positive case
  {
    i = 0;
    while (data_buf[i] != '.')
      temp = temp * 10 + (data_buf[i++] - 0x30);
    for (j = 0; j < num; j++)
      temp = temp * 10 + (data_buf[++i] - 0x30);
    for (j = 0; j < num; j++)
      temp = temp / 10 ;
  }
  return temp;
}
void rec_init()//initial GPS
{
  Wire.beginTransmission(GPSAddress);
  WireSend(0xff);//To send data address
  Wire.endTransmission();

  Wire.beginTransmission(GPSAddress);
  Wire.requestFrom(GPSAddress, 10); //Require 10 bytes read from the GPS device
}
char ID()//Receive the statement ID
{
  char i = 0;
  char value[7] = {
    '$', 'G', 'P', 'G', 'G', 'A', ','
  };//To receive the ID content of GPS statements
  char buff[7] = {
    '0', '0', '0', '0', '0', '0', '0'
  };

  while (1)
  {
    rec_init();//Receive data initialization
    while (Wire.available())
    {
      buff[i] = WireRead();//Receive serial data
      if (buff[i] == value[i]) //Contrast the correct ID
      {
        i++;
        if (i == 7)
        {
          Wire.endTransmission();//End of receiving
          return 1;//Receiving returns 1

        }
      }
      else
        i = 0;
    }
    Wire.endTransmission();//End receiving
  }
}
void UTC()//Time information
{
  char i = 0, flag = 0;
  char value[7] = {
    '$', 'G', 'P', 'G', 'G', 'A', ','
  };
  char buff[7] = {
    '0', '0', '0', '0', '0', '0', '0'
  };
  char time[9] = {
    '0', '0', '0', '0', '0', '0', '0', '0', '0'
  };//Storage time data

  double t = 0.0;

  while (1)
  {
    rec_init();
    while (Wire.available())
    {
      if (!flag)
      {
        buff[i] = WireRead();
        if (buff[i] == value[i])
        {
          i++;
          if (i == 7)
          {
            i = 0;
            flag = 1;
          }
        }
        else
          i = 0;
      }
      else
      {
        time[i] = WireRead();
        i++;
        if (i == 9)
        {
          t = Datatransfer(time, 2); //Converts floating-point data
          t = t + 80000.00; //To convert time to Beijing time
          Serial.println(t);//The time data output
          Wire.endTransmission();
          return;
        }
      }
    }
    Wire.endTransmission();
  }
}
void rec_data(char *buff, char num1, char num2) //Receive data function
{ //*buff：Receive data array；num1：Number of commas ；num2：The   length of the array
  char i = 0, count = 0;

  if (ID())
  {
    while (1)
    {
      rec_init();
      while (Wire.available())
      {
        buff[i] = WireRead();
        if (count != num1)
        {
          if (buff[i] == ',')
            count++;
        }
        else
        {
          i++;
          if (i == num2)
          {
            Wire.endTransmission();
            return;
          }
        }
      }
      Wire.endTransmission();
    }
  }
}
void latitude()//Latitude information
{
  char lat[10] = {
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'
  };//Store the latitude data
  rec_data(lat, 1 , 10); //Receive the latitude data
  Serial.println(Datatransfer(lat, 5), 5); //output
}
void lat_dir()//Latitude direction information
{
  char dir[1] = {'0'}; //Store latitude direction data

  rec_data(dir, 2, 1); //Receive latitude direction data
  printlnByte(dir[0]);//output latitude direction information
}
void  longitude()//Longitude information
{
  char lon[11] = {
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'
  };//Store longitude data
  rec_data(lon, 3, 11); //Receive the longitude data
  Serial.println(Datatransfer(lon, 5), 5); //out put date
}
void lon_dir()//Longitude direction information

{
  char dir[1] = {'0'};
  rec_data(dir, 4, 1);
  printlnByte(dir[0]);//output latitude direction information
}
void altitude()//Altitude information
{
  char i = 0, count = 0;
  char alt[8] = {
    '0', '0', '0', '0', '0', '0', '0', '0'
  };

  if (ID())
  {
    while (1)
    {
      rec_init();
      while (Wire.available())
      {
        alt[i] = WireRead();
        if (count != 8)
        {
          if (alt[i] == ',')
            count++;
        }
        else
        {
          if (alt[i] == ',')
          {
            Serial.println(Datatransfer(alt, 1), 1);
            Wire.endTransmission();
            return;
          }
          else
            i++;
        }
      }
      Wire.endTransmission();
    }
  }
}
void setup()
{
  //
  Serial.begin(9600);//set baud rate
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("DFRobot DFRduino GPS Shield v1.0");
  Serial.println("$GPGGA statement information: ");
  Wire.begin();//IIC Initialize
  Serial.println("$GPGGA statement information: ");
}
float getTemp (int pin) {
  float adc, volt, temp;
  int sample = 100;
  for (int i = 0; i < 100; i++) {
    adc += analogRead(pin);
  }
  adc = adc / sample;
  volt = (adc * 3.3 / 1023) / 2 * 3;
  temp = (volt - 1.25 ) / 0.005;

  return temp;
}
void loop()
{
  Serial.println("Masuk LOOP");
  while (1)
  {
    Serial.print("Temperature :");
    Serial.print(getTemp(A0));
    Serial.print(" C   ");
    Serial.print("UTC:");
    UTC();
    Serial.print("Lat:");
    latitude();
    Serial.print("Dir:");
    lat_dir();
    Serial.print("Lon:");
    longitude();
    Serial.print("Dir:");
    lon_dir();
    Serial.print("Alt:");
    altitude();
    Serial.println(' ');
    Serial.println(' ');
  }
}
