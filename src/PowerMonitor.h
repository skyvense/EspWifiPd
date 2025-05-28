#ifndef POWER_MONITOR_H
#define POWER_MONITOR_H

#include <Wire.h>
#include <INA3221.h>

// Define channel constants
#define INA3221_CH1 0
#define INA3221_CH2 1
#define INA3221_CH3 2

// 使用不同的I2C引脚
#define I2C_SDA 4 
#define I2C_SCL 5

class PowerMonitor {
private:
    INA3221 ina3221;
    bool initialized;
    const float shuntResistance = 0.025; // 0.025 ohm shunt resistors

    // 扫描I2C设备
    void scanI2CDevices() {
        Serial.println("\n=== Scanning I2C Devices ===");
        byte error, address;
        int deviceCount = 0;
        
        for(address = 1; address < 127; address++) {
            Wire.beginTransmission(address);
            error = Wire.endTransmission();
            
            if (error == 0) {
                Serial.print("I2C device found at address 0x");
                if (address < 16) {
                    Serial.print("0");
                }
                Serial.print(address, HEX);
                
                // 显示已知设备的名称
                switch(address) {
                    case 0x40:
                        Serial.println(" (INA3221)");
                        break;
                    case 0x3C:
                        Serial.println(" (OLED Display)");
                        break;
                    case 0x68:
                        Serial.println(" (RTC DS3231)");
                        break;
                    case 0x57:
                        Serial.println(" (EEPROM AT24C32)");
                        break;
                    default:
                        Serial.println(" (Unknown device)");
                }
                deviceCount++;
            }
        }
        
        if (deviceCount == 0) {
            Serial.println("No I2C devices found!");
        } else {
            Serial.print("Found ");
            Serial.print(deviceCount);
            Serial.println(" device(s)");
        }
        Serial.println("=== I2C Scan Complete ===\n");
    }

public:
    PowerMonitor() : ina3221(0x40, &Wire), initialized(false) {}

    bool begin() {
        Serial.println("\n=== Initializing I2C Power Monitor ===");
        
        // 初始化I2C
        Serial.print("Initializing I2C on pins SDA:");
        Serial.print(I2C_SDA);
        Serial.print(" SCL:");
        Serial.println(I2C_SCL);
        
        Wire.begin(I2C_SDA, I2C_SCL);
        
        // 检查I2C设备是否存在
        if (!ina3221.isConnected()) {
            Serial.println("ERROR: INA3221 not found at address 0x40");
            return false;
        }
        
        // Initialize INA3221
        Serial.println("Initializing INA3221...");
        if (!ina3221.begin()) {
            Serial.println("ERROR: Failed to initialize INA3221");
            return false;
        }
        
        // 配置INA3221
        // 设置分流电阻
        for (int ch = 0; ch < 3; ch++) {
            ina3221.setShuntR(ch, shuntResistance);
        }
        
        // 设置转换时间和平均模式
        ina3221.setBusVoltageConversionTime(4); // 1.1ms
        ina3221.setShuntVoltageConversionTime(4); // 1.1ms
        ina3221.setAverage(4); // 128 samples
        
        // 设置工作模式
        ina3221.setModeShuntBusContinuous(); // 连续测量模式
        
        // 验证设备ID
        uint16_t manufID = ina3221.getManufacturerID();
        uint16_t dieID = ina3221.getDieID();
        
        if (manufID == 0x5449 && dieID == 0x3220) {
            Serial.println("SUCCESS: INA3221 ID verification passed");
            Serial.print("Manufacturer ID: 0x");
            Serial.println(manufID, HEX);
            Serial.print("Die ID: 0x");
            Serial.println(dieID, HEX);
        } else {
            Serial.println("ERROR: INA3221 ID verification failed");
            Serial.print("Manufacturer ID: 0x");
            Serial.println(manufID, HEX);
            Serial.print("Die ID: 0x");
            Serial.println(dieID, HEX);
            return false;
        }
        
        // 打印初始配置
        Serial.println("\nInitial INA3221 Configuration:");
        for (int ch = 0; ch < 3; ch++) {
            float shunt_voltage = ina3221.getShuntVoltage_mV(ch);
            float bus_voltage = ina3221.getBusVoltage_mV(ch);
            float current = ina3221.getCurrent_mA(ch);
            
            Serial.print("Channel ");
            Serial.print(ch + 1);
            Serial.print(" - Shunt Voltage: ");
            Serial.print(shunt_voltage, 3);
            Serial.print(" mV, Bus Voltage: ");
            Serial.print(bus_voltage, 3);
            Serial.print(" mV, Current: ");
            Serial.print(current, 3);
            Serial.println(" mA");
        }
        Serial.println();
        
        initialized = true;
        Serial.println("=== I2C Power Monitor Initialization Complete ===\n");
        return true;
    }

    bool isInitialized() const {
        return initialized;
    }

    float getCurrent_mA(int channel) {
        if (!initialized || channel < 0 || channel > 2) return 0.0;
        // 交换通道1和3
        int actual_channel = channel;
        if (channel == 0) actual_channel = 2;
        else if (channel == 2) actual_channel = 0;
        
        return ina3221.getCurrent_mA(actual_channel);
    }

    float getBusVoltage_V(int channel) {
        if (!initialized || channel < 0 || channel > 2) return 0.0;
        // 交换通道1和3
        int actual_channel = channel;
        if (channel == 0) actual_channel = 2;
        else if (channel == 2) actual_channel = 0;
        
        return ina3221.getBusVoltage_mV(actual_channel) / 1000.0; // Convert mV to V
    }

    float getPower_mW(int channel) {
        if (!initialized || channel < 0 || channel > 2) return 0.0;
        // 交换通道1和3
        int actual_channel = channel;
        if (channel == 0) actual_channel = 2;
        else if (channel == 2) actual_channel = 0;
        
        return ina3221.getPower_mW(actual_channel);
    }

    float getShuntVoltage_mV(int channel) {
        if (!initialized || channel < 0 || channel > 2) return 0.0;
        // 交换通道1和3
        int actual_channel = channel;
        if (channel == 0) actual_channel = 2;
        else if (channel == 2) actual_channel = 0;
        
        return ina3221.getShuntVoltage_mV(actual_channel);
    }

    // 打印所有通道的电压电流信息
    void printPowerInfo() {
        if (!initialized) {
            Serial.println("Power Monitor not initialized!");
            return;
        }

        Serial.println("\n=== Power Monitor Readings ===");
        for (int ch = 0; ch < 3; ch++) {
            float voltage = getBusVoltage_V(ch); // V
            float current = getCurrent_mA(ch); // mA
            float power = getPower_mW(ch); // mW
            
            Serial.print("Channel ");
            Serial.print(ch + 1);
            Serial.println(":");
            Serial.print("  Bus Voltage: ");
            Serial.print(voltage, 3);
            Serial.println(" V");
            Serial.print("  Current: ");
            Serial.print(current, 3);
            Serial.println(" mA");
            Serial.print("  Power: ");
            Serial.print(power, 3);
            Serial.println(" mW");
        }
        Serial.println("===========================\n");
    }
};

#endif // POWER_MONITOR_H 