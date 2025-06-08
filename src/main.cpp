#include <Arduino.h>
#include <I2S.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <EasyLed.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

#include "EspSmartWifi.h"
#include "WebServer.h"
#include "PowerMonitor.h"
#include "Display.h"

//#define PIN        D8

#define STATUS_LED  D4
#define RGB_LED_PIN 13  // GPIO13
#define NUM_LEDS    1   // LED数量
#define BRIGHTNESS  50  // 亮度（0-255）

Adafruit_NeoPixel pixels(NUM_LEDS, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);

// Global variables
EasyLed led(STATUS_LED, EasyLed::ActiveLevel::Low, EasyLed::State::Off);
EspSmartWifi wifi(led);
Display display;
VoltageCtl voltageCtl;
WebServer webServer(wifi, led, display, voltageCtl);
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

// 添加电压切换相关的变量
uint8_t voltageLevels[] = {VOLTAGE_5V, VOLTAGE_9V, VOLTAGE_12V, VOLTAGE_15V, VOLTAGE_20V};  // 电压等级
int currentVoltageIndex = 0;  // 当前电压等级索引

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
                    // 切换到下一个电压等级
                    currentVoltageIndex = (currentVoltageIndex + 1) % (sizeof(voltageLevels) / sizeof(voltageLevels[0]));
                    uint8_t newVoltageLevel = voltageLevels[currentVoltageIndex];
                    
                    // 设置新的电压
                    if (voltageCtl.setVoltage(newVoltageLevel)) {
                        // 显示当前电压等级
                        Serial.print("Switching to voltage level: ");
                        Serial.println(newVoltageLevel);
                        
                        // 闪烁LED指示电压切换
                        led.flash(1, 100, 100, 0, 0);
                    } else {
                        Serial.println("Failed to set voltage level");
                    }
                }
            }
            lastButtonPress = millis();
        }
    }
    lastButtonState = currentButtonState;
}

void setup() {
    Serial.begin(115200);
    Serial.setRxBufferSize(2048);

    // 初始化NeoPixel
    pixels.begin();
    pixels.setBrightness(BRIGHTNESS);
    pixels.show(); // 初始化时关闭所有LED
    
    // 初始化电源监控
    if (!powerMonitor.begin()) {
        Serial.println("Failed to initialize power monitor!");
    }

    // 初始化OLED显示
    if (!display.begin()) {
        Serial.println("Failed to initialize OLED display!");
    }

    wifi.initFS();
    wifi.ConnectWifi();
    wifi.DisplayIP();
    
    voltageCtl.begin();
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

    // 检查WiFi连接状态，内部会delay(1000)如果没连上Wi-Fi
    wifi.WiFiWatchDog();
    
    // 处理WebServer请求
    webServer.handleClient();

    // 更新OLED显示
    display.update();

    if (loop_count % 100 == 0)
    {
        // 获取电压和电流值
        float current = powerMonitor.getCurrent_mA();
        float voltage = powerMonitor.getBusVoltage_V();
        
        // 根据实际电压区间设置RGB颜色
        uint32_t color = pixels.Color(0, 0, 0); // 默认熄灭
        if (voltage >= 4.6 && voltage <= 5.4) {
            color = pixels.Color(0, 255, 0); // 绿色
        } else if (voltage >= 8.6 && voltage <= 9.4) {
            color = pixels.Color(0, 0, 255); // 蓝色
        } else if (voltage >= 11.6 && voltage <= 12.4) {
            color = pixels.Color(128, 0, 128); // 紫色
        } else if (voltage >= 14.6 && voltage <= 15.4) {
            color = pixels.Color(255, 0, 0); // 红色
        } else if (voltage >= 19.6 && voltage <= 20.4) {
            color = pixels.Color(255, 255, 255); // 白色
        }

        // 亮度随电流变化
        uint8_t brightness = map(constrain(current, 0, 1000), 0, 1000, 10, 255);
        pixels.setBrightness(brightness);
        pixels.setPixelColor(0, color);
        pixels.show();

        // 电压异常时STATUS_LED快速闪烁
        float setTarget = 0;
        switch (voltageCtl.getCurrentVoltage()) {
            case VOLTAGE_5V:  setTarget = 5.0; break;
            case VOLTAGE_9V:  setTarget = 9.0; break;
            case VOLTAGE_12V: setTarget = 12.0; break;
            case VOLTAGE_15V: setTarget = 15.0; break;
            case VOLTAGE_20V: setTarget = 20.0; break;
            default: setTarget = 0; break;
        }
        if (fabs(voltage - setTarget) > 0.4) {
            led.flash(10, 50, 50, 0, 0); // 快速闪烁10次
        } else {
            led.off();
        }
    }
    
    // 如果WiFi和MQTT都连接成功
    if (WiFi.status() == WL_CONNECTED && mqtt.connected()) 
    {
        // 定期处理MQTT消息
        mqtt.loop();
        loop_count++;
        if (loop_count % 10000 == 0) 
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