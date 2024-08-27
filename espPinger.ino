#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <ESPping.h>

const char* ssid     = "SSID";
const char* password = "password";

const IPAddress remote_ip(8, 8, 8, 8);

LiquidCrystal_I2C lcd(0x27,16,2);
int times[8*5];  // response times = 8 LCD chars * 5 column per char


/*
 * Write new response time into rotating array.
 */
void writeTime(int time)
{
  // rotate times
  for (int i = (8*5)-1; i > 0; i--)
  {
    times[i] = times[i-1];
  }

  // write the last one to begin of array
  times[0] = time;
}


/*
 * Create LCD graph using custom char.
 */
void createGraphChar(int charIdx) {

  int startIdx = charIdx * 5;        // start index in times array
  byte graphChar[7];                 // char rows

  for(int i=6; i>=0; i--) {           // loop over char's rows from bottom to top

    int limitTime = ((7-i) * 10);     // time limit for actual row

    byte row = 
        (times[startIdx] > limitTime ? 16 : 0)      // 5. column/pixel of actual row
      + (times[startIdx + 1] > limitTime ? 8 : 0)   // 4. column/pixel of actual row
      + (times[startIdx + 2] > limitTime ? 4 : 0)   // ...
      + (times[startIdx + 3] > limitTime ? 2 : 0)   // ...
      + (times[startIdx + 4] > limitTime ? 1 : 0);  // 1. column/pixel of actual row

    graphChar[i] = row;               // add row to char
  }

  lcd.createChar(charIdx, graphChar); // set char to LCD
  lcd.setCursor(charIdx + 8, 1);      // set cursor
  lcd.write(byte(charIdx));           // print char
}


/*
 * Loop over 8 lcd custom chars to plot a graph.
 */
void printGraph(){
  for(int i=0; i<8; i++){
    createGraphChar(i);
  }
}


/*
 * Main arduino setup method.
 */
void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.clear();         
  lcd.backlight();
  
  lcd.setCursor(0,0);
  lcd.print("Connecting WiFi");
  lcd.setCursor(0,1);
  
  WiFi.begin(ssid, password);  
    
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    lcd.print(".");
  }

  lcd.clear();         
  lcd.setCursor(0,0);
  lcd.print("WiFi connected");
  lcd.setCursor(0,1);
  lcd.print(WiFi.localIP());
  delay(1000);
}


/*
 * Main arduino loop method
 */
void loop() {

  bool pingResult = Ping.ping(remote_ip, 1);

  Serial.println("Pinging " + remote_ip.toString());
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.println("Ping " + remote_ip.toString() + "           "); // Default print leave on LCD some characters mapped to custom chars and it is drawed with a graph. We need to clean whole line.
  lcd.setCursor(0, 1);

  if (pingResult > 0)
  {
    int minTime = (int)Ping.minTime();

    Serial.println("response time: " + String(minTime) + "ms");
    lcd.print(String(minTime) + "ms");
    writeTime(minTime);
  } 
  else 
  {
    Serial.println("Error !");
    lcd.print("timeout");
    writeTime(255);
  }

  printGraph();

  delay(1000);
}
