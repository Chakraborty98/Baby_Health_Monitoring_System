#include <Wire.h>
#include <LiquidCrystal_I2C_Hangul.h>
LiquidCrystal_I2C_Hangul lcd(0x27, 16, 2);
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#define ONE_WIRE_BUS 12

const int buzzer = 13;
const int IR1 = 14;
const int soundsensor = 4;
const int waterpin = 32;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float temperatureC;
int waterlevel;
boolean soundval;
boolean ir1val;

String padstatus = "";
String soundstatus = "";
String movementstatus = "";
String from_name = "Ronnie";
String chat_id = "1195817521";

#define WIFI_SSID "Abhishek"                                         // input your home or public wifi name 
#define WIFI_PASSWORD "12345678"

#define BOTtoken "                                               "   // give your personal BOTtoken ID
#define CHAT_ID "             "                                      // give your personal telegram id number

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;
void setup()
{
  Serial.begin(115200);
  Serial.println("WELCOME");
  delay(200);
  pinMode(buzzer, OUTPUT);
  pinMode(IR1, INPUT);
  pinMode(soundsensor, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  delay(500);

  bot.sendMessage(CHAT_ID, "WELCOME. Bot started up", "");
  buzzeron();
  delay(4000);
  buzzeron();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("    WELCOME   ");
  delay(1000);
  sensors.begin();

}

void buzzeron()
{
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
  delay(200);
}

void readIRSensor()
{
  ir1val = digitalRead(IR1);
  if (ir1val == LOW)
  {
    Serial.println("MOVEMENT Detected");
    delay(200);
    lcd.setCursor(0, 0);
    lcd.print("Movement Detected  ");
    delay(200);
    movementstatus = "DETECTED";
    bot.sendMessage(CHAT_ID, "ALERT.. Baby Movement Detected.\n", "");
    sendupdates();
  }

  else
  {
    Serial.println("NO MOVEMENT");
    delay(200);
    lcd.setCursor(0, 0);
    lcd.print("No Movement     ");
    delay(500);
    movementstatus = "NOT DETECTED";
  }
}


void readSoundSensor()
{
  soundval = digitalRead(soundsensor);
  if (soundval == HIGH)
  {
    Serial.println("SOUND Detected");
    delay(200);
    lcd.setCursor(0, 0);
    lcd.print("Sound Detected ");
    delay(500);
    soundstatus = "CRY";
    bot.sendMessage(CHAT_ID, "ALERT.. Baby is Crying.\n", "");
    sendupdates();
  }

  else
  {
    Serial.println("SOUND NOT Detected");
    delay(200);
    lcd.setCursor(0, 0);
    lcd.print("No Sound Detected");
    delay(500);
    soundstatus = "SILENT";
  }
}


void readtemperature()
{
  sensors.requestTemperatures();
  temperatureC = sensors.getTempFByIndex(0);
  Serial.print("Temperature is: ");
  Serial.println(temperatureC);
  if (temperatureC > 98)
  {
    Serial.println("Temperature is High");
    digitalWrite(buzzer, HIGH);
    delay(300);
    digitalWrite(buzzer, LOW);
    delay(100);
    lcd.setCursor(0, 0);
    lcd.print("High Temperature");
    delay(500);
    bot.sendMessage(CHAT_ID, "ALERT.. Baby Temperature is High.\n", "");
    sendupdates();
  }

}


void readwaterlevel()
{
  waterlevel = analogRead(waterpin);
  //int waterlevel = 4095 - waterval;
  Serial.print("water Level:");
  Serial.println(waterlevel);
  delay(100);


  if (waterlevel > 100)
  {
    digitalWrite(buzzer, HIGH);
    delay(1000);
    digitalWrite(buzzer, LOW);
    delay(100);
    Serial.println("WETPAD");
    lcd.setCursor(0, 0);
    lcd.print("Pad   is   Wet");
    delay(500);
    padstatus = "WET";
    bot.sendMessage(CHAT_ID, "ALERT.. Baby's PAD is WET. \n", "");
    sendupdates();

  }
  else
  {
    Serial.println("DRYPAD");
    lcd.setCursor(0, 0);
    lcd.print("Pad   is   Dry");
    delay(500);
    padstatus = "DRY";
  }
}

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    // Chat id of the requester
    chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "/status to receive sensor updates. \n";

      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/request") {
      sendupdates();

    }
  }
}

void sendupdates()
{
  String updates = "Hi, " + from_name + ".\n";
  updates += "Here is your sensor values.\n\n";
  updates += "Temperature: ";
  updates += String(temperatureC);
  updates += " Deg F \n";
  updates += "Movement: ";
  updates += movementstatus;
  updates += " \n";
  updates += "Sound: ";
  updates += soundstatus;
  updates += " \n";
  updates += "Pad: ";
  updates += padstatus;
  updates += " \n";

  bot.sendMessage(chat_id, updates, "");
}

void readtelegram() {
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}

void loop()
{
  readtelegram();
  readwaterlevel();
  readtemperature();
  readIRSensor();
  readSoundSensor();
}
