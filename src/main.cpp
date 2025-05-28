#include <Arduino.h>
#include <I2S.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <EasyLed.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
//#include <Adafruit_NeoPixel.h>

#include "EspSmartWifi.h"
#include "WebServer.h"
#include "PowerMonitor.h"
#include "Display.h"

//#define PIN        D8

#define STATUS_LED  D4

// Global variables
EasyLed led(STATUS_LED, EasyLed::ActiveLevel::Low, EasyLed::State::Off);
EspSmartWifi wifi(led);
Display display;
WebServer webServer(wifi, led, display);
PowerMonitor powerMonitor;
PubSubClient mqtt(wifi.client);

// How many NeoPixels are attached to the Arduino?
//#define NUMPIXELS 1 // Popular NeoPixel ring size

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
//Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// 添加按钮状态变量
bool lastButtonState = HIGH;
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 50;

// 解析MQTT服务器地址
bool parseMQTTServer(const String& serverUrl, String& host, int& port, String& username, String& password) {
    // 检查URL格式
    if (!serverUrl.startsWith("mqtt://")) {
        Serial.println("Invalid MQTT server URL format");
        return false;
    }

    // 移除 mqtt:// 前缀
    String url = serverUrl.substring(7);
    
    // 检查是否有认证信息
    int atPos = url.indexOf('@');
    if (atPos != -1) {
        // 有认证信息
        String auth = url.substring(0, atPos);
        int colonPos = auth.indexOf(':');
        if (colonPos != -1) {
            username = auth.substring(0, colonPos);
            password = auth.substring(colonPos + 1);
        }
        url = url.substring(atPos + 1);
    }

    // 检查是否有端口号
    int colonPos = url.indexOf(':');
    if (colonPos != -1) {
        host = url.substring(0, colonPos);
        port = url.substring(colonPos + 1).toInt();
    } else {
        host = url;
        port = 1883; // 默认端口
    }

    return true;
}

// 连接MQTT服务器
bool connectMQTT() {
    const Config& config = wifi.getConfig();
    String host;
    int port;
    String username;
    String password;
    
    if (parseMQTTServer(config.Server, host, port, username, password)) {
        mqtt.setServer(host.c_str(), port);
        mqtt.setBufferSize(2048);
        
        // 使用lambda函数作为回调
        mqtt.setCallback([](char* topic, byte* payload, unsigned int length) {
            const Config& config = wifi.getConfig();
            if (strcmp(topic, config.Topic.c_str()) == 0) {
                String message;
                for (int i = 0; i < length; i++) {
                    message += (char)payload[i];
                }
                Serial.print(message.c_str());
            }
        });
        
        // 生成客户端ID
        String clientId = "EspRouterPower" + String(ESP.getChipId(), HEX);
        
        // 如果有认证信息，设置用户名和密码
        if (username.length() > 0) {
            return mqtt.connect(clientId.c_str(), username.c_str(), password.c_str());
        } else {
            return mqtt.connect(clientId.c_str());
        }
    }
    return false;
}

// 发送MQTT消息的辅助函数
void publishMQTT(const char* topic, const char* message) {
    if (WiFi.status() == WL_CONNECTED && mqtt.connected()) {
        mqtt.publish(topic, message);
        mqtt.loop();  // 确保消息被处理
    }
}

// 发送电源监控数据的函数
void publishPowerData() {
    StaticJsonDocument<512> doc;
    
    Serial.println("\n=== Power Monitor Readings ===");
    // 获取三个通道的数据
    for (int i = 0; i < 1; i++) {
        float current = powerMonitor.getCurrent_mA();
        float voltage = powerMonitor.getBusVoltage_V();
        float power = powerMonitor.getPower_mW();
        
        // 打印到串口
        Serial.print("Channel ");
        Serial.print(i + 1);
        Serial.println(":");
        Serial.print("  Current: ");
        Serial.print(current, 3);
        Serial.println(" mA");
        Serial.print("  Voltage: ");
        Serial.print(voltage, 3);
        Serial.println(" V");
        Serial.print("  Power: ");
        Serial.print(power, 3);
        Serial.println(" mW");
        
        // 添加到JSON文档
        JsonObject channel = doc.createNestedObject("channel" + String(i + 1));
        channel["current"] = current;
        channel["voltage"] = voltage;
        channel["power"] = power;
        
    }
    Serial.println("===========================\n");
    
    char buffer[512];
    serializeJson(doc, buffer);
    const Config& config = wifi.getConfig();
    publishMQTT(config.Topic.c_str(), buffer);
}

// 处理按钮输入的函数
void checkButton() {
    bool currentButtonState = digitalRead(BUTTON_PIN);
    
    // 检测按钮按下（防抖）
    if (currentButtonState != lastButtonState) {
        if (millis() - lastButtonPress > debounceDelay) {
            if (currentButtonState == LOW) {  // 按钮按下
                // 记录按下时间
                unsigned long pressStartTime = millis();
                
                // 等待按钮释放
                while (digitalRead(BUTTON_PIN) == LOW) {
                    delay(50);  // 每50ms检查一次
                    
                    // 如果长按超过3秒
                    if (millis() - pressStartTime > 3000) {
                        // 清除配置文件
                        if (SPIFFS.remove("/config.json")) {
                            Serial.println("Configuration cleared, restarting...");
                            led.flash(5, 100, 100, 0, 0);  // 快速闪烁5次表示清除成功
                            delay(1000);
                            ESP.restart();
                        }
                        break;
                    }
                }
                
                // 如果按下时间小于3秒，执行短按操作
                if (millis() - pressStartTime < 3000) {
                    webServer.handleButtonPress();
                }
            }
            lastButtonPress = millis();
        }
    }
    lastButtonState = currentButtonState;
}

void setup() {
    Serial.begin(115200);  // 使用硬件串口，同时用于调试和Air780E通信
    Serial.setRxBufferSize(2048); // 增加串口接收缓冲区大小

    // 初始化电源监控
    if (!powerMonitor.begin()) {
        Serial.println("Failed to initialize power monitor!");
    }

    // 初始化OLED显示
    if (!display.begin()) {
        Serial.println("Failed to initialize OLED display!");
    }

    //pixels.begin();

    wifi.initFS();
    wifi.ConnectWifi();
    wifi.DisplayIP();
    
    // 启动WebServer
    webServer.begin();
    
    // 连接MQTT服务器
    if (!connectMQTT()) {
        Serial.println("Failed to connect to MQTT server");
    }

    delay(500); // 等待MQTT连接

    // 初始化按钮引脚
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

int loop_count = 0;

void loop() {
    // 处理按钮输入（放在最前面，确保不会被阻塞）
    checkButton();

    wifi.WiFiWatchDog();
    
    // 处理WebServer请求
    webServer.handleClient();

    // 更新OLED显示
    display.update();

    // 如果WiFi和MQTT都连接成功
    if (WiFi.status() == WL_CONNECTED && mqtt.connected()) 
    {
        // 定期处理MQTT消息
        mqtt.loop();
        loop_count++;
        if (loop_count % 1000 == 0) 
        {
            // 每秒发布一次电源数据
            publishPowerData();
            led.flash(1, 25, 25, 0, 0);
        }
    } 
    else 
    {
        // 如果WiFi或MQTT连接断开，尝试重新连接
        if (wifi.isAPMode())
        {
            loop_count++;
            if (loop_count % 2000 == 0) 
            {
                led.flash(1, 10, 50, 0, 0);
                delay(10);
            }
        } 
        else 
        {
            if (!mqtt.connected())
            {
                loop_count++;
                if (loop_count % 5000 == 0) 
                {
                    if (!connectMQTT()) 
                    {
                        Serial.println("Failed to reconnect to MQTT server");
                    }
                    led.flash(2, 50, 50, 0, 0);
                }
            }
        }
       
    }

    delay(1);  // 减少主循环延迟
}