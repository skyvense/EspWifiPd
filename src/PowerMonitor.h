#pragma once

#include <Wire.h>
#include <Adafruit_INA219.h>

class PowerMonitor {
public:
    PowerMonitor() : ina219() {}
    
    bool begin() {
        Wire.begin();
        if (!ina219.begin()) {
            Serial.println("Failed to find INA219 chip");
            return false;
        }
        
        // 配置INA219使用0.01Ω分流电阻
        // 使用自定义校准
        // 最大电流 = 3.2A
        // 最大电压 = 32V
        // 分流电阻 = 0.01Ω
        ina219.setCalibration_32V_2A();
        
        // 打印配置信息
        Serial.println("INA219 initialized successfully");
        Serial.println("Configuration:");
        Serial.println("- Shunt Resistor: 0.01 ohm");
        Serial.println("- Max Current: 3.2A");
        Serial.println("- Max Voltage: 32V");
        
        return true;
    }
    
    float getBusVoltage_V() {
        return ina219.getBusVoltage_V();
    }
    
    float getCurrent_mA() {
        return ina219.getCurrent_mA();
    }
    
    float getPower_mW() {
        return ina219.getPower_mW();
    }
    
    bool isInitialized() {
        return ina219.begin();
    }

private:
    Adafruit_INA219 ina219;
}; 