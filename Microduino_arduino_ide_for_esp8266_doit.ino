#include <ESP8266WiFi.h>
#include "DHT.h"


#define DHTPIN 13    // 连接gpio 13
#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

#define u8 unsigned char

const char* ssid     = "ssid";
const char* password = "password";

const char* host = "iot.doit.am"; //物联网平台
const int httpPort = 8810;

const char* streamId   = "uid";
const char* privateKey = "api key";
char ser[64];
char str[512];
WiFiClient client;// 使用WiFi客户端类创建TCP连接

unsigned long MS_TIMER = 0;
unsigned long lastMSTimer = 0;
String comdata = "";
String recDataStr ="";
char flag = false;
bool bConnected = false;

//希尔排序，消除跳变
void shellsort( u8 k[],int n)
{
    int i, j, flag ,gap = n;
     int tmp;
    while(gap > 1){
    gap = gap/2;               
    do{                        
        flag = 0;
        for(i=0;i<=n-gap;i++){
            j = i + gap;
            if(k[i]<k[j]){
                tmp = k[i];
                k[i] = k[j];
                k[j] = tmp;
                flag = 1;
                }
            }
         }while(flag !=0);
    }
}

void setup() {
  Serial.begin(115200); 
  dht.begin(); 
   
  MS_TIMER = millis();    
  
  WiFi.disconnect();
  // 首先，我们连接到WiFi网络
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(50);
  Serial.print("connecting to ");
  Serial.println(host);
  
}

unsigned long lastTick = 0;

void loop() {
  delay(1);
  // 使用WiFi客户端类创建TCP连接
  if (bConnected == false)
  {
  if (!client.connect(host, httpPort))
  {
    Serial.println("connection failed"); 
    delay(5000); 
    return;  
  }
   bConnected = true;
    Serial.println("connection ok");
  }          

  if (flag == false)
  {
    if (millis() - MS_TIMER > 5000)
    {
      MS_TIMER = millis();
    }
  }

   if (millis() - lastTick > 5000)
  {
    lastTick = millis();
    static bool first_flag = true;
    if (first_flag)
    {
      first_flag = false;
      sprintf(str, "cmd=subscribe&topic=uid\r\n");//使用注册uid登陆
      client.print(str);
      return;
    }

    int i;
    u8 a[11], b[11],h,t;             
   
    for(i=0;i<=11;i++){    
    delay(2000);  
    u8 m_h = dht.readHumidity(); 
    u8 m_t = dht.readTemperature(); 
    a[i] = {m_h};   
    b[i] = {m_t};
    }
    shellsort(a,11);
    h = a[5];
    shellsort(b,11);    
    t = b[5];
    Serial.println(t);
    Serial.println(h);    
    
    sprintf(ser, "Temp: %d'C, Humi: %d%.", t, h);    
    Serial.println(ser);
     
    sprintf(str, "cmd=upload&device_name=Humi&data=%d&device_name=Temp&data=%d&uid=uid&key=%s\r\n", h, t, privateKey);
    client.print(str);    //实测30秒数据更新一次
    
  }
  
  if (client.available())
  {
    //串口读取网站回文
    String recDataStr = client.readStringUntil('\n');
    Serial.println(recDataStr);
    
    }     
}
