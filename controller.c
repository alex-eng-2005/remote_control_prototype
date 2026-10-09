//Put this code inside arduino
//Then delete the controller.c in this folder when finished
#include "Freenove_WS2812_Lib_for_ESP32.h"
#include "LiquidCrystal_I2C.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <string.h>


//Buzzer
#define buzzer 7

//Poten
#define X_RANGE 9
#define Y_RANGE 10

//LCD Screen
#define SDA 41
#define SCL 42

//STRIP
#define LEDS_COUNT 8
#define LEDS_PIN 6
#define CHANNEL 0

//Buttons
int buttons[] = {48,47,21, 45};
//Letters
//(.): Enter word
//(_): Enter on the text
//(<): Backspace
char* letters[] = {
    "a", "b", "c", "d", "e", "f", "g",
    "h", "i", "j", "k", "l", "m", "n",
    "o", "p", "q", "r", "s", "t", "u",
    "v", "w", "x", "y", "z",
    "A", "B", "C", "D", "E", "F", "G",
    "H", "I", "J", "K", "L", "M", "N",
    "O", "P", "Q", "R", "S", "T", "U",
    "V", "W", "X", "Y", "Z", ".", "_","<"
};
//LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);
//Message
String message = "";
//Index
int message_index = 0;
//Index
int cursor_index = 0;
//Strip index
int strip_index = 0;
//Size
const int SIZE = sizeof(letters) / sizeof(letters[0]);
//Wifi
const char* SSID = ""; //Enter wifi name
//Password
const char* PASSWORD = ""; //Enter password
//Other computer IP Address
const char* IP_MESSAGE = ""; //Enter computer ip address
//Access the x and y address
const char* IP_X_AND_Y = "";
//Click
const char* IP_CLICK = "";
//Colors
int delay_colors[3] = {0,255, 0};
//Not Connected
int not_connected[3] = {255, 0, 0};
//Strip
Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);
void setup()
{
    //Serial Begin
    Serial.begin(115200);
    //Screen
    Wire.begin(SDA, SCL);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(cursor_index,0);
    lcd.print(letters[cursor_index]);
    //Buttons
    for(int i = 0; i < 4; i++)
    {
      pinMode(buttons[i], INPUT_PULLUP);
    }
    //Buzzeer
    pinMode(buzzer, OUTPUT);

    //Sets the wifi
    WiFi.mode(WIFI_STA);
    //Sets the ssid and password
    WiFi.begin(SSID, PASSWORD);

    strip.begin();

    //Turns on the strips
    while(strip_index < LEDS_COUNT)
    {
     //Adds to the strip
      strip.setLedColorData(strip_index, delay_colors[0], delay_colors[1], delay_colors[2]);
      strip.show();
      //Index++
      strip_index += 1;
      delay(500);
    }
    strip_index = 0;

    //Turns off the strips
    for(int i = 0; i < LEDS_COUNT; i++)
    {
      //Adds to the strip
      strip.setLedColorData(i, 0,0,0);
      strip.show();
    }

    //Connecting the WIFI
    Serial.println("Connecting to the WIFI");
    //Connects to the password
    while(WiFi.status() != WL_CONNECTED)
    {
      Serial.println("NOT CONNECTED");

      //Adds to the strip
      strip.setLedColorData(strip_index, not_connected[0], not_connected[1], not_connected[2]);
      strip.show();
      strip_index += 1;
      
      //Delays
      delay(500);
      Serial.print(".");

      if(strip_index == 8)
      {
        //Turns off the strips
        for(int i = 0; i < LEDS_COUNT; i++)
        {
          //Adds to the strip
          strip.setLedColorData(i, 0,0,0);
          strip.show();
        }
        strip_index = 0;
      }
      
    }
    sendMessage("IT HAS BEEN SENT");
}

void loop()
{
  //Button check
  keyboard();
  //X and Y range
  xAndY();
}

void xAndY()
{
  static unsigned long lastSend = 0;
  if (millis() - lastSend < 1000)
  {
    return;
  }
  lastSend = millis();

  int x = analogRead(X_RANGE);
  int y = analogRead(Y_RANGE);
  sendXAndY(x,y);
  //delay(100);
}

void keyboard()
{
  //Buttons
  int button1 = buttons[0];
  int button2 = buttons[1];
  int button3 = buttons[2];
  int button4 = buttons[3];

  if(digitalRead(button1) == LOW)
  {
    message_index--;
    if(message_index < 0)
    {
      message_index = SIZE - 1;
    }
    Serial.println(message_index);
    lcd.setCursor(cursor_index,0);
    lcd.print(letters[message_index]);
    delay(100);
  }
  if(digitalRead(button2) == LOW)
  {
    if(letters[message_index] == ".")
    {
      //Sends the message
      sendMessage(message);
      //Clears everything
      lcd.clear();
      lcd.setCursor(0,0);
      cursor_index = 0;
      message = "";
      message_index = 0;
      //Shows new message
      lcd.print(letters[message_index]);
      //Buzzer
      digitalWrite(buzzer, HIGH);
      delay(150);
      digitalWrite(buzzer, LOW);
    }
    else if(letters[message_index] == "<")
    {
      if(message.length() > 0)
      {
        cursor_index -= 1;
        message.remove(message.length() - 1);
        lcd.clear();
        lcd.setCursor(cursor_index, 0);
        lcd.print(message);
      }
    }
    else
    {
      message += letters[message_index];
      //New lcd message
      lcd.setCursor(0,0);
      lcd.print(message);
      //Outputs the new message
      lcd.setCursor(cursor_index + 1, 0);
      lcd.print(letters[message_index]);
      cursor_index++;
    }
     delay(100);
  }
  if(digitalRead(button3) == LOW)
  {
    message_index++;
    if(message_index == SIZE)
    {
      message_index = 0;
    }
    lcd.setCursor(cursor_index,0);
    lcd.print(letters[message_index]);
    delay(100);
  }
  if(digitalRead(button4) == LOW)
  {
    wasClick();
    delay(100);
  }
}

void sendXAndY(int x, int y)
{
  if(WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.setConnectTimeout(100);
    http.setTimeout(100);
    http.begin(IP_X_AND_Y);
    http.addHeader("Content-Type","application/json");

    //JSON Data
    String jsonData = "{\"x\":" + String(x) + ",\"y\":" + String(y) + "}";
    //Response code
    int responseCode = http.POST(jsonData);
    if(responseCode > 0)
    {
      String response = http.getString();
      Serial.println("FastAPI response:");
      Serial.println(response);
      Serial.print("It is sent");
    }
    else
    {
      Serial.println("It is not sent 2");
    }

    http.end();
  }

}
void sendMessage(String text)
{
  if(WiFi.status() == WL_CONNECTED)
  {
   
    HTTPClient http;
    http.begin(IP_MESSAGE);
    http.addHeader("Content-Type", "application/json");

    //JSON Data
    String jsonData = "{\"text\":\"" + text + "\"}";
    //Our response code
    int responseCode = http.POST(jsonData);

    if(responseCode > 0)
    {
      String response = http.getString();
      Serial.println("FastAPI response:");
      Serial.println(response);
      Serial.print("It is sent");
    }
    else
    {
      Serial.println("It is not sent 2");
    }

    http.end();
  }
}

void wasClick()
{
  if(WiFi.status() == WL_CONNECTED)
  {
   
    HTTPClient http;
    http.begin(IP_CLICK);
    http.addHeader("Content-Type", "application/json");

    //JSON Data
    String jsonData = "{\"click\":true}";
    //Our response code
    int responseCode = http.POST(jsonData);

    if(responseCode > 0)
    {
      String response = http.getString();
      Serial.println("FastAPI response:");
      Serial.println(response);
      Serial.print("It is sent");
    }
    else
    {
      Serial.println("It is not sent 3");
    }

    http.end();
  }
}