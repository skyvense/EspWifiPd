#pragma once
#include <Arduino.h>
#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

struct Config {
  String SSID = "1";
  String Passwd = "1";
  String Server = "mqtt://username:passwd@mqtt.server";  // MQTT服务器地址，支持认证
  String Topic = "/espRouterPower/power";  // MQTT主题
};

struct EMPTY_SERIAL
{
  void println(const char *){}
  void println(String){}
  void printf(const char *, ...){}
  void print(const char *){}
  //void print(Printable) {}
  void begin(int){}
  void end(){}
};
//_EMPTY_SERIAL _EMPTY_SERIAL;
//#define Serial_debug  _EMPTY_SERIAL
#define Serial_debug  Serial

class EasyLed;
class EspSmartWifi
{
private:
    EasyLed &led_;
    fs::File root;
    Config _config;
    bool _isAPMode;
    bool cachedRelayStates[4] = {false};  // 缓存继电器状态
    bool statesLoaded = false;  // 标记是否已加载状态

    void BaseConfig();
    void StartAPMode();

    bool LoadConfig();
    bool SaveConfig();
    
    // 继电器状态相关方法
    bool loadRelayStatesFromFlash();  // 从flash加载状态到缓存
    bool saveRelayStatesToFlash();    // 将缓存保存到flash

public:
    EspSmartWifi(EasyLed &led):
    led_(led), _isAPMode(false)
    {
    }
    ~EspSmartWifi(){

    }

    void initFS();
    bool WiFiWatchDog();
    void ConnectWifi();
    void DisplayIP();
    
    // HTTP client methods
    String httpGet(const String& path);
    const Config& getConfig() const { return _config; }
    WiFiClient client;

    // 继电器状态相关方法
    bool updateRelayState(int index, bool state);
    bool getRelayStates(bool states[4]);
    void syncRelayStates();  // 同步继电器状态到硬件
    bool SaveConfig(Config config);
    // 获取AP模式状态
    bool isAPMode() const { return _isAPMode; }
};




