#include <ESP8266WiFi.h>
#include "DHT.h"


#define DHTPIN 13    // what digital pin we're connected to

#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

#define u8 unsigned char

const char* ssid     = "zmhm";
const char* password = "12312300";

const char* host = "iot.doit.am";//物联网平台
const int httpPort = 8810;

const char* streamId   = "topdog";
const char* privateKey = "503e7fe9f6b1b99d108fdb4d1a2cdbf0";
char ser[64];
char str[512];
WiFiClient client;// 使用WiFi客户端类创建TCP连接
//反向控制：
unsigned long MS_TIMER = 0;
unsigned long lastMSTimer = 0;
String comdata = "";
String recDataStr ="";
char flag = false;
bool bConnected = false;

void shellsort( u8 k[],int n)
{
    int i, j, flag ,gap = n;
     int tmp;
    while(gap > 1){
    gap = gap/2;                /*增量缩小，每次减半*/
    do{                        /*子序列应用冒泡排序*/
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
      sprintf(str, "cmd=subscribe&topic=topdog\r\n");//注册用户名，以让后面的反向控制得以是实现
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
     
    sprintf(str, "cmd=upload&device_name=Humi&data=%d&device_name=Temp&data=%d&uid=topdog&key=%s\r\n", h, t, privateKey);
    client.print(str);    //实测30秒数据更新一次
    
  }
  
  if (client.available())
  {
    //读并处理
    // R读取服务器的应答的所有行，并把它们打印到串口
    String recDataStr = client.readStringUntil('\n');//必须要接收这个应答，否则读数无法正常进行！！！！！！
    Serial.println(recDataStr);
    
    }     
}
