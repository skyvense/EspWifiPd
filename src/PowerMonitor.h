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
        
        // 配置INA219
        ina219.setCalibration_32V_1A();
        Serial.println("INA219 initialized successfully");
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