#include <ThreeWire.h>
#include <RtcDS1302.h>
ThreeWire myWire(4,5,6); // DAT,CLK,RST時間模組接腳設定
RtcDS1302<ThreeWire> Rtc(myWire);
//以上時間模組設定

#include <TimerOne.h>
#include "TM1637.h"
#define CLK 2//4PIN7段顯示器接腳設定
#define DIO 3
TM1637 tm1637(CLK,DIO);
int8_t TimeDisp[] = {0x00,0x00,0x00,0x00};
//以上4PIN7段顯示器*4設定

#include <LiquidCrystal_PCF8574.h>//LCD設定
#include <Wire.h>
LiquidCrystal_PCF8574 lcd(0x27);//不是0x27就是0x3F
//以上LCD設定

#include "DHT.h"
#define DHTPIN 7     
#define DHTTYPE DHT11   
//#define DHTTYPE DHT22  
//#define DHTTYPE DHT21   
DHT dht(DHTPIN, DHTTYPE);
//以上溫溼度感測器設定

#include "MQ7.h"
#define A_PIN A3
#define VOLTAGE 5
MQ7 mq7(A_PIN, VOLTAGE);
//以上一氧化碳感測器設定

const int buttonPin = 8;   // 滑動開關街腳

const int coled = 13;   // 一氧化碳指示燈
const int co = 9;       //偵測一氧化碳感測器的輸出接腳

#include "Adafruit_Keypad.h"
const byte ROWS = 4; // 列數(橫的)
const byte COLS = 4; // 行數(直的)
long int Q1[7]={0,0,0,0,0,0,0};//0~4輸入5位數字  5:+-*/運算符號 6總和
long int Q2[7]={0,0,0,0,0,0,0};//0~4輸入5位數字  5:=運算符號 6總和
int n,m,k,a,b=0; //n第一個數字計數器 m為第個數字計數器，k求總和的0有幾個
int number=0; //計算是否為第一或第二數
//鍵盤上每一個按鍵的名稱
char keys[ROWS][COLS] = 
{
  {1,2,3,'A'},
  {4,5,6,'B'},
  {7,8,9,'C'},
  {'=',10,'#','D'}
};
byte rowPins[ROWS] = {12, 11, 10, A0}; //定義列的腳位
byte colPins[COLS] = {A1, A2, 4, 5}; //定義行的腳位
//初始化鍵盤
Adafruit_Keypad customKeypad = Adafruit_Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);
//以上為鍵盤計算機設定

void setup () 
{
    Serial.begin(9600);
    while (!Serial) {;}
    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);
    Rtc.Begin();
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();
    if (!Rtc.IsDateTimeValid()) 
    {
        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }
    if (Rtc.GetIsWriteProtected())
    {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }
    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }
    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
        Rtc.SetDateTime(compiled);
    }
    else if (now == compiled) 
    {
      Serial.println("RTC is the same as compile time! (not expected but all is fine)");
      Rtc.SetDateTime(compiled);
    }
    //以上為時間模組設定，顯示模組時間和電腦時間的新舊，檢查時間模組腳位有沒有接好
    
    tm1637.set();
    tm1637.init();
    //以上為4PIN7段顯示器*4設定，應該只是初始化
    
    int error;
    Serial.println("LCD...");
    Serial.println("Dose: check for LCD");
    Wire.begin();
    Wire.beginTransmission(0x27); 
    error = Wire.endTransmission();
    Serial.print("Error: ");
    Serial.print(error);
    if (error == 0) 
    {Serial.println(": LCD found.");} 
    else 
    {Serial.println(": LCD not found.");}
    //以上為LCD設定，顯示找不找得到LCD
    
    Serial.println(F("DHTxx test!"));
    dht.begin();
    //以上為啟動溫溼度感測器
    
    Serial.println("Calibrating MQ7");
    mq7.calibrate();
    Serial.println("Calibration done!");
    //以上為一氧化炭感測器
    
    pinMode(buttonPin, INPUT);
    //以上按鈕設接腳設定輸入
    
    pinMode(coled, OUTPUT);
    pinMode(co, INPUT);
    //以上為一氧化碳安全警示燈設定輸出和輸入

    customKeypad.begin();
    Serial.println("KeyPad Test...");
    //以上為啟動鍵盤
}

void loop () 
{
    RtcDateTime now = Rtc.GetDateTime();
    printDateTime(now);
    Serial.println();
    Rtc.SetDateTime(now);
    if (!now.IsValid())
    {Serial.println("RTC lost confidence in the DateTime!");}
    //以上為時間模組設定，如果時間找不到顯示錯誤
    
    tm1637.display(TimeDisp);
    //以上為4PIN7段顯示器*4設定，應該只是初始化
    
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float f = dht.readTemperature(true);
    if (isnan(h) || isnan(t) || isnan(f)) 
    {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
    float hif = dht.computeHeatIndex(f, h);
    float hic = dht.computeHeatIndex(t, h, false);
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C "));
    Serial.println();
    //以上為溫溼度設定
    
    Serial.print("CO PPM = "); Serial.println(mq7.readPpm());
    //以上為CO顯示
    
    //Serial.println(digitalRead(co));
    //Serial.println(digitalRead(buttonPin));
    //Serial.println(digitalRead(buttonPin1));
    //各項數值顯示以利檢查
    
    lcd.setBacklight(255);//背光(亮度)
    if (digitalRead(buttonPin) == HIGH && a==0){lcd.clear();}
    lcd.home();
    if (digitalRead(buttonPin) == LOW )//滑動開關下為LOW
    { 
      lcd.clear();
      Rtc.SetDateTime(now);
      lcd.setCursor(0,0);//開始行數
      lcd.clear();
      lcd.print("Humidity:");
      lcd.print(h);
      lcd.print("%");
      lcd.setCursor(0,1);
      lcd.print("Temperat:");
      lcd.print(t);
      lcd.print("C");
      delay(5000);
      a=0;
      //以上為LCD顯示溫溼度
    }
    else
    {
      //lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("CO=");
      lcd.print(mq7.readPpm());
      lcd.print(" PPM");
      calculation();
      delay(1000);
      a=1;
    }
    
    if(digitalRead(co)==LOW)
    {digitalWrite(coled, HIGH);}
    else
    {digitalWrite(coled, LOW);}
    //以上為一氧化碳安全警示燈
}

void calculation()//計算機函式
{
  // 開始偵測使用者的按鍵狀態
  customKeypad.tick();
  //判斷按了哪一個鍵
  while(customKeypad.available())
  {
    keypadEvent e = customKeypad.read();
    if(e.bit.EVENT == KEY_JUST_PRESSED)
    {
      if(Q1[6]!=0&&Q1[5]==10)
      {
        lcd.clear();
        for(int i=0;i<7;i=i+1)
        {
          Q1[i]=0;
        }
      }
      if(Q1[6]!=0&&Q2[6]!=0)
      {
        lcd.clear();
        for(int i=0;i<7;i=i+1)
        {
          Q1[i]=0;
          Q2[i]=0;
        }
      }
      if(number==0)
      {
        Q1[n]=int(e.bit.KEY);
        Serial.println(int(Q1[n]));
        for(int i=0;i<=10;i=i+1)
        {
          lcd.setCursor(n,1);
          if(Q1[n]==10)
          {
            lcd.print("sqrt");
            lcd.print("=");
          }
          else if(Q1[n]==i){lcd.print( Q1[n]);}
        }
        if(Q1[0]==10)
        {
          Serial.println("ERROR");
          Serial.println("INPUT AGAIN");
          lcd.clear();
          lcd.setCursor(0,1);
          for(int i=0;i<=6;i=i+1){Q1[i]=0;}
          n=6;
        }
        if(Q1[n]==10)
        {
          if(n<5){Q1[5]=Q1[n];Q1[n]=0;n=6;break;}
          if(n==5){n=6;break;}
        }
        if(Q1[n]=='#')
        {
          for(int i=0;i<7;i=i+1)
          {Q1[i]=0;}
          lcd.clear();
          lcd.setCursor(0,1);
          n=0;m=0;number=0;break;
        }
        //Serial.println(int(Q1[5]));
        if(Q1[n]=='A'||Q1[n]=='B'||Q1[n]=='C'||Q1[n]=='D')
        {
          if(Q1[0]=='A'||Q1[0]=='B'||Q1[0]=='C'||Q1[0]=='D'||Q1[0]=='=')
          {
              Serial.println("ERROR");
              Serial.println("INPUT AGAIN");
              lcd.clear();
              lcd.setCursor(0,1);
              for(int i=0;i<=6;i=i+1){Q1[i]=0;}
          }
          if(Q1[n]=='A'){Serial.println("+");number=1;lcd.setCursor(n,1);lcd.print("+");}
          if(Q1[n]=='B'){Serial.println("-");number=1;lcd.setCursor(n,1);lcd.print("-");}
          if(Q1[n]=='C'){Serial.println("*");number=1;lcd.setCursor(n,1);lcd.print("*");}
          if(Q1[n]=='D'){Serial.println("/");number=1;lcd.setCursor(n,1);lcd.print("/");}
          if(n<5){Q1[5]=Q1[n];Q1[n]=0;n=6;break;}
          if(n==5){n=6;break;}
        } 
        if(Q1[n]=='=')
        {
          Serial.println("ERROR");
          Serial.println("INPUT AGAIN");
          for(int i=0;i<=6;i=i+1){Q1[i]=0;}
          lcd.clear();
          lcd.setCursor(0,1);
          n=6;break;
        } 
        if(n<=6){ n=n+1;}
        if(n>6){n=6;}
        if(n==6)
        {
          for(int i=0;i<=9;i=i+1)
          {
            if(Q1[5]==i||Q1[5]=='=')
            {
              Serial.println("ERROR");
              Serial.println("INPUT AGAIN");
              for(int i=0;i<=6;i=i+1){Q1[i]=0;}
              lcd.clear();
              lcd.setCursor(0,1);
              n=6;break;
            }
          }
        } 
      }
      //////////////////////////以上顯示第一數字/////////////////////////////////
      if(number==1)
      {
        if(b==0)
        {
          for(int i=0;i<6;i=i+1)
          {
            if(Q1[i]!=0){b=b+1;}
          }
          m=0;
        }
        Q2[m]=int(e.bit.KEY);
        Serial.println(int(Q2[m]));
        for(int i=0;i<10;i=i+1)
        {
          if(Q2[m]==i)
          {
            lcd.setCursor(b+m,1);
            lcd.print(Q2[m]);
          }
        }
        if(Q2[m]=='#')
        {
          for(int i=0;i<7;i=i+1)
          {Q2[i]=0;}
          lcd.clear();
          lcd.setCursor(0,1);
          n=0;m=0;number=0;b=0;break;
        }
        if(Q2[m]=='A'||Q2[m]=='B'||Q2[m]=='C'||Q2[m]=='D')
        {
          Serial.println("ERROR");
          Serial.println("INPUT AGAIN");
          for(int i=0;i<=4;i=i+1){Q2[m]=0;}
          lcd.clear();
          lcd.setCursor(0,1);
          for(int i=0;i<5;i=i+1)
          {
            if(Q1[i]!=0)
            {lcd.print(Q1[i]);}
          }
          if(Q1[5]=='A'){lcd.print("+");}
          if(Q1[5]=='B'){lcd.print("-");}
          if(Q1[5]=='C'){lcd.print("*");}
          if(Q1[5]=='D'){lcd.print("/");}
          m=0; b=0;
        }
        if(Q2[0]=='=')
        {
          Serial.println("ERROR");
          Serial.println("INPUT AGAIN");
          for(int i=0;i<=4;i=i+1){Q2[m]=0;}
          lcd.clear();
          lcd.setCursor(0,1);
          for(int i=0;i<5;i=i+1)
          {
            if(Q1[i]!=0)
            {lcd.print(Q1[i]);}
          }
          if(Q1[5]=='A'){lcd.print("+");}
          if(Q1[5]=='B'){lcd.print("-");}
          if(Q1[5]=='C'){lcd.print("*");}
          if(Q1[5]=='D'){lcd.print("/");}
          m=0; b=0;
        }
        if(Q2[m]=='=')
        {
          lcd.setCursor(b+m,1);
          lcd.print("=");
          if(m<5){Q2[5]=Q2[m];Q2[m]=0;m=6;b=0;break;}
          if(m==5){m=6;break;}
        } 
        if(m<=6){ m=m+1;}
        if(m>6){ m=6;}
        if(m==6)
        {
          for(int i=0;i<=9;i=i+1)
          {
            if(Q2[5]==i||Q2[5]=='A'||Q2[5]=='B'||Q2[5]=='C'||Q2[5]=='D')
            {
              Serial.println("ERROR");
              Serial.println("INPUT AGAIN");
              lcd.clear();
              lcd.setCursor(0,1);
              for(int i=0;i<5;i=i+1)
              {
                if(Q1[i]!=0)
                {lcd.print(Q1[i]);}
              }
              if(Q1[5]=='A'){lcd.print("+");}
              if(Q1[5]=='B'){lcd.print("-");}
              if(Q1[5]=='C'){lcd.print("*");}
              if(Q1[5]=='D'){lcd.print("/");}
              m=0; b=0;
              for(int i=0;i<=6;i=i+1){Q2[i]=0;}
             // m=6;break;
            }
          }
        }
      }
    }
  }
  //////////////////////////以上顯示第二數字/////////////////////////////////
    if(n==6)//算第一數總和
    {
      for(int i=0;i<5;i=i+1)
      {if(Q1[i]==0){k=k+1;}}
      Q1[6]=Q1[0]*10000+Q1[1]*1000+Q1[2]*100+Q1[3]*10+Q1[4];
      if(k==4){Q1[6]=Q1[6]/10000;k=0;}
      if(k==3){Q1[6]=Q1[6]/1000;k=0;}
      if(k==2){Q1[6]=Q1[6]/100;k=0;}
      if(k==1){Q1[6]=Q1[6]/10;k=0;}
      if(Q1[5]==10){lcd.print(sqrt(Q1[6]));}
      Serial.println(Q1[6]);
      n=0;
      k=0;
    }

    if(m==6)//算第二數總和和運算結果
    {
      for(int i=0;i<5;i=i+1)
      {if(Q2[i]==0){k=k+1;}}
      Q2[6]=Q2[0]*10000+Q2[1]*1000+Q2[2]*100+Q2[3]*10+Q2[4];
      if(k==4){Q2[6]=Q2[6]/10000;k=0;}
      if(k==3){Q2[6]=Q2[6]/1000;k=0;}
      if(k==2){Q2[6]=Q2[6]/100;k=0;}
      if(k==1){Q2[6]=Q2[6]/10;k=0;}
      Serial.println(Q2[6]);
      for(int i=0;i<6;i=i+1)
      {
        if(Q2[i]=='=')
        {
          if(Q1[5]=='A'){Serial.println(Q1[6]+Q2[6]);lcd.print(Q1[6]+Q2[6]);break;}
          if(Q1[5]=='B'){Serial.println(Q1[6]-Q2[6]);lcd.print(Q1[6]-Q2[6]);break;}
          if(Q1[5]=='C'){Serial.println(Q1[6]*Q2[6]);lcd.print(Q1[6]*Q2[6]);break;}
          if(Q1[5]=='D'){Serial.println((Q1[6]*1.0)/(Q2[6]/1.0),2);lcd.print((Q1[6]*1.0)/(Q2[6]/1.0),2);break;}
        }
      }
      m=0;
      k=0;
      b=0;
      number=0;  
    } 
}

#define countof(a) (sizeof(a) / sizeof(a[0]))//時間模組計算時間的函式+7段顯示器顯示時間
void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];
    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.print(datestring);//以上為時間顯示用
    TimeDisp[0] = dt.Hour() / 10;//時十位數
    TimeDisp[1] = dt.Hour() % 10;//時個位數
    TimeDisp[2] = dt.Minute() / 10;//分十位數
    TimeDisp[3] = dt.Minute() % 10;//分個位數
    tm1637.point(POINT_ON);//顯示:恆亮
}
