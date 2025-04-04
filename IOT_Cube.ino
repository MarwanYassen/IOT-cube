/*
Email: iot.cube.2022@gmail.com
Pass : theiotcube
*/

#define BLYNK_TEMPLATE_ID "TMPLFTGLZnXm"
#define BLYNK_DEVICE_NAME "The IoT Cube"
#define BLYNK_AUTH_TOKEN "WUdMJ0BJ2COPsFIQ6sKmyr93UTSUDkwZ"
#define BLYNK_FIRMWARE_VERSION        "0.1.0"
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_NODE_MCU_BOARD

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

///////////////////////////////////////////////////
#define RL 1.5 //Calibration resistor
///////////////////////////////////////////////////

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <stdlib.h>
#include <math.h>
#include <SimpleTimer.h> 
#include <DHT.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

DHT dht;
SimpleTimer timer;

const int MQ4 = A0;
int Methane; 
int Slope = 0;
int Expected_Temperature = 0;
bool Connected2Blynk = false;

///////////////////////////////////////////////////
float ppm;
float r0=10; //10k = estimat for 1000ppm // datasheet says 10k..60k
// calibration in known concentration is needed !
float ratio;
float rs;
int i;
///////////////////////////////////////////////////

char auth[] = "WUdMJ0BJ2COPsFIQ6sKmyr93UTSUDkwZ";
/*char ssid[] = "HUAWEI_B311_8DB9";
char pass[] = "Tatmb2RA5aY";
*/

char ssid[] = "E-Just";
char pass[] = "ahmedloay";

/*
char ssid[] = "E-JUST";
char pass[] = "Ejust@Staff2022";
*/
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


void setup()
{

   Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.clearDisplay();
  timer.setInterval(11000L, CheckConnection); // check if still connected every 11 seconds
  Serial.println("\nStarted");
  MyWiFi();
  delay(100);
  dht.setup(D5);
  pinMode(D3, OUTPUT);
  pinMode(D0, OUTPUT);
}

void loop() {

while(WiFi.status() == WL_CONNECTED){
  Blynk.run();
  digitalWrite(D0, HIGH);
    pinMode(D3, OUTPUT);
    for (int fadeValue = 0 ; fadeValue <= 255; fadeValue = fadeValue+5) {
			analogWrite(D3, fadeValue);
			delay(10);
		}
		for (int fadeValue = 255 ; fadeValue >= 0; fadeValue = fadeValue-5) {
			analogWrite(D3, fadeValue);
			delay(10);
		} 


     delay(dht.getMinimumSamplingPeriod()); /* Delay of amount equal to sampling period */
     float humidity = dht.getHumidity();/* Get humidity value */
     float temperature = dht.getTemperature();

   
///////////////////////////////////////////////////
  Methane = analogRead(MQ4); 
  rs=((RL * (1023 - Methane) / Methane));
  ratio=rs/r0;
  ppm=1021*pow((ratio),-2.7887);
 int  ppb = ppm/1000 ;
///////////////////////////////////////////////////

int CH4average = 0;
 for (int i=0; i < 2; i++) {
 CH4average = CH4average + Methane;
 }
 CH4average = CH4average/2;
 int TEMPaverage = 0;
 for (int i=0; i < 2; i++) {
 TEMPaverage = TEMPaverage + temperature;
 }
 TEMPaverage = TEMPaverage/2;
Slope = CH4average / TEMPaverage ;
Expected_Temperature = 35.943558767092384+0.03142697854651222*ppb ;

     Blynk.virtualWrite(V0, humidity);
     Blynk.virtualWrite(V1, temperature  );
     Blynk.virtualWrite(V2, ppm);
     Blynk.virtualWrite(V3, Slope);
     Blynk.virtualWrite(V4, Expected_Temperature);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Cloud:    Connected");
  display.println("");
  display.print("Temp:     ");
  display.print(temperature, 1);
  display.print(" C");
  display.println("");
  display.print("CH4:      ");
  display.print(ppm, 1);
  display.print(" ppm");
  display.println("");
  display.print("Humidity: ");
  display.print(humidity, 1);
  display.print(" %");

  display.display();

}


 while(WiFi.status() != WL_CONNECTED){
  pinMode(D0, OUTPUT);
  digitalWrite(D3, HIGH);
  digitalWrite(D0, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(100);                      // wait for a second
  digitalWrite(D0, LOW);   // turn the LED off by making the voltage LOW
  delay(100);  
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Cloud: Disconnected");
  display.println("");
  display.print("Temp:     ");
  display.print("NAN ");
  display.print("C");
  display.println("");
  display.print("CH4:      ");
  display.print("NAN ");
  display.print("ppm");
  display.println("");
  display.print("Humidity: ");
  display.print("NAN ");
  display.print("%");
  display.display();

}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void MyWiFi(){
  int mytimeout = millis() / 1000;

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if((millis() / 1000) > mytimeout + 3){ // try for less than 4 seconds to connect to WiFi router
      break;
    }
  }

  if(WiFi.status() == WL_CONNECTED){  
    Serial.print("\nIP address: ");
    Serial.println(WiFi.localIP()); 
  }
  else{
    Serial.println("\nCheck Router ");    
  }
  Blynk.config(auth);
  Connected2Blynk = Blynk.connect(1000);  // 1000 is a timeout of 3333 milliseconds 
  mytimeout = millis() / 1000;
  while (Blynk.connect(1000) == false) { 
    if((millis() / 1000) > mytimeout + 3){ // try for less than 4 seconds
      break;
    }
  }  
}

void CheckConnection(){
  Connected2Blynk = Blynk.connected();
  if(!Connected2Blynk){
    Serial.println("Not connected to Blynk server");
    MyWiFi();  
  }
  else{
    Serial.println("Still connected to Blynk server");    
  }
}



