// Motor
// Motor A Connections
int enA = 9;
int in1 = 8; 
int in2 = 7;
// Motor B Connections
int enB = 3;
int in3 = 5;
int in4 = 4;
int speed1 = 0;
unsigned long minSpinTime = 3000; 
unsigned long lastTimeMotorStateChanged = 0;
#define SPEED_PIN A7
 
// Ultrasonic
int trigPin = 6;    
int echoPin = 10;    
long duration, cm, inches;
long pastDist = 0;
unsigned long minUpdateTime = 300; 
unsigned long lastTimeUpdated = 0;
float targetTempIndex = 20;
float targetDist = 55;

// DHT
#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN A4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

float hic = 0;
float hicOld = 0;

// LCD Display
#include <LiquidCrystal_74HC595.h>
#define DS 11
#define SHCP 13
#define STCP 12
#define RS 1
#define E 3
#define D4 4
#define D5 5
#define D6 6
#define D7 7

LiquidCrystal_74HC595 lcd(DS, SHCP, STCP, RS, E, D4, D5, D6, D7);
String regMode = "Regular";
String ultraMode = "Ultrasonic";
String pwmPrint;
String tempPrint;
#define BACKLIGHT_PIN A3

// Button
#define BUTTON_PIN 2
byte lastButtonState = LOW;
int counter = 0;
// Debounce
unsigned long debounceDuration = 50; 
unsigned long lastTimeButtonStateChanged = 0;

// LED pins
int red_light_pin= A0;
int green_light_pin = A1;
int blue_light_pin = A2;

void setup() 
{
  Serial.begin(9600);
  //DHT
  dht.begin();
  
  // lcd
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  
  //Set all the motor control pins to 
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  
  // Turn off motors - Initial state
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);

  //Ultrasonic
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //Button
  pinMode(BUTTON_PIN, INPUT);
  
  //LED
  pinMode(red_light_pin, OUTPUT);
  pinMode(green_light_pin, OUTPUT);
  pinMode(blue_light_pin, OUTPUT);

  //
  
}

void loop() 
{

  if (millis() - lastTimeButtonStateChanged > debounceDuration) 
  {
      byte buttonState = digitalRead(BUTTON_PIN);
      if (buttonState != lastButtonState) //when new mode is chosen
      {
        lastTimeButtonStateChanged = millis();
        lastButtonState = buttonState;
        if (buttonState == LOW) 
        {
          counter++;
          if(counter == 3)
            counter = 0;
          Serial.println("Button released:  ");
          Serial.println(counter);
          if(counter == 0)
          {
            RGBColor(0,0,0);
            digitalWrite(BACKLIGHT_PIN, LOW);
            lcd.noDisplay();
            sleepFans();
          }
          else if(counter == 1) //add a brief display period
          {
            RGBColor(0,255,0);
            dhtControl();
            ultrasonic();
            Serial.println(String(hic) + "  " + String(cm));
            if(hic > targetTempIndex && cm < targetDist)
            {
              digitalWrite(BACKLIGHT_PIN, HIGH);
              lcd.display();
              lastTimeMotorStateChanged = millis();
              spinFans();
              Serial.print("spin fans");
            }
            else
            {
              digitalWrite(BACKLIGHT_PIN, LOW);
              lcd.noDisplay();
              sleepFans();
              Serial.println("off");
            }
          }
          else if(counter == 2)
          {
            RGBColor(0,0,255);
            dhtControl();
            ultrasonic();
            digitalWrite(BACKLIGHT_PIN, HIGH);
            lcd.display();
            spinFans();
          }
        }
      }
      else //if mode does not change
      {
        if(counter != 0)
        {
          dhtControl();
          ultrasonic();
          Serial.println("else state: " + String(hic) + "  " + String(cm));
        }
        if(counter == 1)
        {
          if(hic <= targetTempIndex || cm >=  targetDist)
          {
            if (millis() - lastTimeMotorStateChanged > minSpinTime) 
            {
              digitalWrite(BACKLIGHT_PIN, LOW);
              lcd.noDisplay(); 
              sleepFans();
            }
            Serial.print("sleep fans temporarily");
          }
          else
          {
            if (millis() - lastTimeMotorStateChanged > minSpinTime) 
            {
              digitalWrite(BACKLIGHT_PIN, LOW);
              lcd.noDisplay(); 
              sleepFans();
            }
            lastTimeMotorStateChanged = millis();
            digitalWrite(BACKLIGHT_PIN, HIGH);
            lcd.display(); 
            spinFans();
          }
        }
        else if(counter == 2)
        {
          spinFans();
        }
      }
  }
}

void sleepFans()
{
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);

  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

  pwmPrint = "S: 0% ";
  lcd.setCursor(10,1);
  lcd.print(pwmPrint);
}
void spinFans()
{
  speed1 = analogRead(SPEED_PIN);
  speed1 = speed1*0.2492668622; 
  Serial.println(speed1);
  pwmPrint = "S: " + String(int(round(speed1/256.0*10)*10)) + "% ";
  lcd.setCursor(9,1);
  lcd.print(pwmPrint);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);

  speed1 = sqrt(speed1*10);
  analogWrite(enB, speed1);
  analogWrite(enA, speed1);
}

void ultrasonic()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
 
  cm = (duration/2) / 29.1;     
  inches = (duration/2) / 74;  
  lcd.setCursor(0,0);
  Serial.print("D: " + String(cm) + "   ");
  if(cm != pastDist && (millis() - lastTimeUpdated) > minUpdateTime)
  { 
    lastTimeUpdated = millis();
    lcd.print("D:" + String(cm) + "   ");
  }
  pastDist = cm;
}

void dhtControl()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) 
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  //float hif = dht.computeHeatIndex(f, h);
  hic = dht.computeHeatIndex(t, h, false);
  tempPrint = "HT: " + String(int(hic)) + "C";
  if(hic != hicOld)
  {
    lcd.setCursor(0,1);
    lcd.print(tempPrint);
  }
  hicOld = hic;
}

void RGBColor(int red_light_value, int green_light_value, int blue_light_value)
 {
  analogWrite(red_light_pin, red_light_value);
  analogWrite(green_light_pin, green_light_value);
  analogWrite(blue_light_pin, blue_light_value);
}
