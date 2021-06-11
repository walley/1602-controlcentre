#include <Arduino.h>
#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>

#define VERSION 0.2
#define SN 221
#define SNOUT 2202
// what digital pin we're connected to
#define DHTPIN 2
#define DHTOUTPIN 2
#define LINETYPE DHT
#define LINEOUTTYPE DS
#define ONE_WIRE_BUS 10

//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);
OneWire one_wire(ONE_WIRE_BUS);
DallasTemperature dallas(&one_wire);

unsigned long interval = 4000;
unsigned long prev_millis = 0;

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

char message_buffer[50];
char data[50];

//uint8_t bell[8]  = {0x4,0xe,0xe,0xe,0x1f,0x0,0x4};
//uint8_t note[8]  = {0x2,0x3,0x2,0xe,0x1e,0xc,0x0};
//uint8_t clockie[8] = {0x0,0xe,0x15,0x17,0x11,0xe,0x0};
uint8_t heart[8] = {0x0,0xa,0x1f,0x1f,0xe,0x4,0x0};
//uint8_t duck[8]  = {0x0,0xc,0x1d,0xf,0xf,0x6,0x0};
//uint8_t check[8] = {0x0,0x1,0x3,0x16,0x1c,0x8,0x0};
//uint8_t cross[8] = {0x0,0x1b,0xe,0x4,0xe,0x1b,0x0};
//uint8_t retarrow[8] = {	0x1,0x1,0x5,0x9,0x1f,0x8,0x4};

byte customChar[8] = {
  0b10000,
  0b10000,
  0b10000,
  0b10110,
  0b01101,
  0b01101,
  0b01101,
  0b01101
};

byte deg_of_c[8] = {
  0b01000,
  0b10100,
  0b01000,
  0b00011,
  0b00100,
  0b00100,
  0b00100,
  0b00011
};

LiquidCrystal_I2C lcd(0x3f, 16, 2); // LCD address, chars and lines

struct value {
  char name[10];
  float value;
};

void create_line(char * type, char * sn, value v[], char count, char * data)
{
  char buf[12];
  char i = 0;

  strcpy(data, "type:");
  strcat(data, type);
  strcat(data, ",");
  strcat(data, "sn:");
  strcat(data, sn);
  strcat(data, ",");

  for (i = 0; i < count; i++) {
    strcat(data, v[i].name);
    strcat(data, ":");
    dtostrf(v[i].value, 5, 2, buf);
    strcat(data, buf);
    strcat(data, ",");
  }

  strcat(data, ";");
}

void displayKeyCodes(void)
{
  uint8_t i = 0;
  lcd.clear();
  lcd.print("Codes 0x");
  lcd.print(i, HEX);
  lcd.print("-0x");
  lcd.print(i+16, HEX);
  lcd.setCursor(0, 1);

  for (int j=0; j<16; j++) {
    lcd.printByte(i+j);
  }

  i += 16;
}

void setup()
{
  //Serial.begin(9600);
  Serial.begin(115200);
  Serial.setTimeout(3000);

  dht.begin();
  dallas.begin();
  lcd.init();
  lcd.backlight();

  //lcd.createChar(0, bell);
  //lcd.createChar(1, note);
  lcd.createChar(2, customChar);
  lcd.createChar(3, heart);
  //lcd.createChar(4, duck);
  //lcd.createChar(5, check);
  //lcd.createChar(6, cross);
  //lcd.createChar(7, retarrow);
  lcd.createChar(8, customChar);
  lcd.createChar(9, deg_of_c);
  lcd.home();

  lcd.print("Control centre");
  lcd.setCursor(0, 1);
  lcd.print("ver. ");
  lcd.print(VERSION);
  lcd.print(" ");
  //lcd.printByte(3);
  lcd.write(3);

  prev_millis = millis();
}

void delay_func()
{
  int i,j = 0;
  int line = 0;
  char c;
  int message_length = 0;

  if (Serial.available()) {
    lcd.clear();
    i = 0;

    while (Serial.available() > 0) {
      c = Serial.read();

      if (i < 50) {
        message_buffer[i++] = c;
      }
    }

    if (i > 0) {
      message_length = i - 1;
      Serial.println("goodlength");
      Serial.println(message_length);
    } else {
      Serial.println("bad length");
      return;
    }

    lcd.setCursor(0, line);

    for (j = 0; j < message_length; j++) {
      if (message_buffer[j] == ':') {
        line = 1;
        lcd.setCursor(0, line);
        continue;
      }

      if (message_buffer[j] == '\n' || message_buffer[j] == ';') {
        line = 0;
        lcd.setCursor(0, line);
        delay(1000);
        lcd.clear();
        continue;
      }

      Serial.print(message_buffer[j]);
      lcd.print(message_buffer[j]);
    }

    Serial.println(".");
    delay(10);
  }
}

void loop()
{
  value vdht[2];
  value vds[1];
  float ds_temp;

  unsigned long curr_millis = millis();

  if ((unsigned long)(curr_millis - prev_millis) < interval) {
    delay_func();
  } else  {
    Serial.println("hi");
    prev_millis = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    dallas.requestTemperatures();
    ds_temp = dallas.getTempCByIndex(0);

    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("t:");
    lcd.print(t);
    lcd.printByte(9);
    lcd.print(ds_temp);
    lcd.printByte(9);
    lcd.setCursor(0, 1);
    lcd.print("h:");
    lcd.print(h);
    lcd.print("%");

    Serial.print("type:DHT,");
    Serial.print("sn:");
    Serial.print(SN);
    Serial.print(",");
    Serial.print("hum:");
    Serial.print(h);
    Serial.print(",");
    Serial.print("temp:");
    Serial.print(t);
    Serial.println(";");

    strcpy(vdht[0].name,"hum");
    vdht[0].value=h;
    strcpy(vdht[1].name,"temp");
    vdht[1].value=t;
    strcpy(vds[0].name,"temp");
    vds[0].value=ds_temp;

    create_line("DHT","2201",vdht,2,data);
    Serial.println(data);
    create_line("DS","2202",vds,1,data);
    Serial.println(data);
  }
}

