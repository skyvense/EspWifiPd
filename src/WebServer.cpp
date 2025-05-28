#include "WebServer.h"
#include <ArduinoJson.h>
#include <FS.h>  // 添加SPIFFS支持

#define BUILD_DATE_STR __DATE__ " " __TIME__

// 定义静态成员变量
const int WebServer::RELAY_PINS[3] = {RELAY_PIN_1, RELAY_PIN_2, RELAY_PIN_3};
const char WebServer::INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP8266 Relay Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        :root {
            --primary-color: #2196F3;
            --primary-dark: #1976D2;
            --success-color: #4CAF50;
            --error-color: #f44336;
            --text-color: #333;
            --text-light: #666;
            --background: #f5f5f5;
            --card-background: #ffffff;
            --border-radius: 8px;
            --shadow: 0 2px 10px rgba(0,0,0,0.1);
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: var(--background);
            color: var(--text-color);
            line-height: 1.6;
            padding: 20px;
        }

        .container {
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
        }

        .header {
            text-align: center;
            margin-bottom: 30px;
            animation: slideDown 0.5s ease-out;
        }

        @keyframes slideDown {
            from {
                opacity: 0;
                transform: translateY(-20px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }

        h1 {
            color: var(--primary-color);
            font-size: 28px;
            margin-bottom: 10px;
        }

        .status {
            color: var(--text-light);
            font-size: 16px;
        }

        .relay-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }

        .relay-card {
            background: var(--card-background);
            border-radius: var(--border-radius);
            padding: 20px;
            box-shadow: var(--shadow);
            transition: transform 0.3s ease;
            animation: fadeIn 0.5s ease-out;
        }

        @keyframes fadeIn {
            from { opacity: 0; }
            to { opacity: 1; }
        }

        .relay-card:hover {
            transform: translateY(-5px);
        }

        .relay-header {
            display: flex;
            align-items: center;
            margin-bottom: 15px;
        }

        .relay-icon {
            width: 40px;
            height: 40px;
            background: var(--primary-color);
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            margin-right: 15px;
        }

        .relay-icon i {
            color: white;
            font-size: 20px;
        }

        .relay-title {
            font-size: 18px;
            font-weight: 500;
            color: var(--text-color);
        }

        .relay-status {
            font-size: 14px;
            color: var(--text-light);
            margin-top: 5px;
        }

        .switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 34px;
        }

        .switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }

        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            transition: .4s;
            border-radius: 34px;
        }

        .slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }

        input:checked + .slider {
            background-color: var(--success-color);
        }

        input:checked + .slider:before {
            transform: translateX(26px);
        }

        .power-info {
            background: var(--card-background);
            border-radius: var(--border-radius);
            padding: 20px;
            box-shadow: var(--shadow);
            margin-top: 30px;
            animation: fadeIn 0.5s ease-out;
        }

        .power-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin-top: 15px;
        }

        .power-item {
            text-align: center;
            padding: 15px;
            background: #f8f9fa;
            border-radius: var(--border-radius);
            transition: transform 0.2s;
        }

        .power-item.monitor {
            cursor: pointer;
        }

        .power-item.monitor:hover {
            transform: scale(1.02);
        }

        .power-value {
            font-size: 24px;
            font-weight: 600;
            color: var(--primary-color);
            margin: 10px 0;
        }

        .power-label {
            font-size: 14px;
            color: var(--text-light);
        }

        .config-link {
            display: inline-block;
            margin-top: 20px;
            padding: 10px 20px;
            background: var(--primary-color);
            color: white;
            text-decoration: none;
            border-radius: var(--border-radius);
            transition: background 0.3s ease;
        }

        .config-link:hover {
            background: var(--primary-dark);
        }

        .footer {
            margin-top: 30px;
            padding-top: 20px;
            border-top: 1px solid #eee;
            display: flex;
            justify-content: space-between;
            align-items: center;
            color: var(--text-light);
            font-size: 14px;
        }

        .upgrade-link {
            color: var(--primary-color);
            text-decoration: none;
            display: flex;
            align-items: center;
            gap: 5px;
        }

        .upgrade-link:hover {
            color: var(--primary-dark);
            text-decoration: underline;
        }

        @media (max-width: 600px) {
            .container {
                padding: 10px;
            }

            h1 {
                font-size: 24px;
            }

            .relay-grid {
                grid-template-columns: 1fr;
            }

            .power-grid {
                grid-template-columns: 1fr;
            }
        }

        /* 添加模态框样式 */
        .modal {
            display: none;
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background-color: rgba(0,0,0,0.5);
            z-index: 1000;
        }

        .modal-content {
            position: relative;
            background-color: white;
            margin: 10% auto;
            padding: 20px;
            width: 80%;
            max-width: 800px;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
        }

        .close {
            position: absolute;
            right: 20px;
            top: 10px;
            font-size: 28px;
            font-weight: bold;
            cursor: pointer;
        }

        .chart-container {
            margin-top: 20px;
            height: 300px;
        }

        .chart-controls {
            margin: 15px 0;
            text-align: right;
        }

        .chart-controls select {
            padding: 5px 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
            background-color: white;
            font-size: 14px;
        }

        .chart-controls select:focus {
            outline: none;
            border-color: var(--primary-color);
        }

        .timer-control {
            margin-top: 10px;
        }
        
        .timer-btn {
            background: var(--primary-color);
            color: white;
            border: none;
            padding: 5px 10px;
            border-radius: 4px;
            cursor: pointer;
            font-size: 14px;
        }
        
        .timer-btn:hover {
            background: var(--primary-dark);
        }
        
        .timer-config {
            margin-top: 20px;
        }
        
        .timer-section {
            margin-bottom: 20px;
        }
        
        .timer-inputs {
            display: flex;
            align-items: center;
            gap: 10px;
            margin-top: 10px;
        }
        
        .timer-inputs input[type="time"] {
            padding: 5px;
            border: 1px solid #ddd;
            border-radius: 4px;
        }

        .timer-list {
            margin-bottom: 20px;
        }
        
        .timer-item {
            background: #f8f9fa;
            padding: 10px;
            margin: 5px 0;
            border-radius: 4px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        
        .timer-item:hover {
            background: #e9ecef;
        }
        
        .timer-info {
            flex-grow: 1;
        }
        
        .timer-actions {
            display: flex;
            gap: 10px;
        }
        
        .timer-btn.delete {
            background: #dc3545;
        }
        
        .timer-btn.delete:hover {
            background: #c82333;
        }
        
        .input-group {
            margin: 10px 0;
        }
        
        .input-group label {
            display: block;
            margin-bottom: 5px;
        }
        
        .input-group input,
        .input-group select {
            width: 100%;
            padding: 8px;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        
        .weekday-buttons {
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
            margin-top: 5px;
        }
        
        .weekday-buttons label {
            display: flex;
            align-items: center;
            gap: 5px;
        }

        .protection-config {
            margin-top: 20px;
        }
        
        .protection-section {
            margin-bottom: 20px;
        }
        
        .protection-inputs {
            display: flex;
            flex-direction: column;
            gap: 15px;
            margin: 15px 0;
        }
        
        .protection-item {
            background: #f8f9fa;
            padding: 10px;
            margin: 5px 0;
            border-radius: 4px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        
        .protection-label {
            font-weight: 500;
        }
        
        .protection-value {
            color: var(--text-light);
        }
        
        .warning {
            color: var(--error-color);
            margin-left: 10px;
        }
    </style>
    <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.4/css/all.min.css" rel="stylesheet">
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>ESP8266 Relay Control</h1>
            <div class="status">Status: Connected</div>
        </div>

        <div class="power-info relay-control">
            <h2>Relay Control</h2>
            <div class="power-grid">
                <div class="power-item">
                    <div class="power-label">Relay 1</div>
                    <div class="power-value">
                        <label class="switch">
                            <input type="checkbox" onchange="toggleRelay(1, this.checked)">
                            <span class="slider"></span>
                        </label>
                    </div>
                    <div class="power-label">Status: <span id="status1">OFF</span></div>
                </div>
                <div class="power-item">
                    <div class="power-label">Relay 2</div>
                    <div class="power-value">
                        <label class="switch">
                            <input type="checkbox" onchange="toggleRelay(2, this.checked)">
                            <span class="slider"></span>
                        </label>
                    </div>
                    <div class="power-label">Status: <span id="status2">OFF</span></div>
                </div>
                <div class="power-item">
                    <div class="power-label">Relay 3</div>
                    <div class="power-value">
                        <label class="switch">
                            <input type="checkbox" onchange="toggleRelay(3, this.checked)">
                            <span class="slider"></span>
                        </label>
                    </div>
                    <div class="power-label">Status: <span id="status3">OFF</span></div>
                </div>
            </div>
        </div>

        <div class="power-info power-monitor">
            <h2>Power Monitoring</h2>
            <div class="power-grid">
                <div class="power-item">
                    <div class="power-label">Channel 1</div>
                    <div class="power-value" id="power1">0.00 W</div>
                    <div class="power-label">Current: <span id="current1">0.00 mA</span></div>
                    <div class="power-label">Voltage: <span id="voltage1">0.00 V</span></div>
                </div>
                <div class="power-item">
                    <div class="power-label">Channel 2</div>
                    <div class="power-value" id="power2">0.00 W</div>
                    <div class="power-label">Current: <span id="current2">0.00 mA</span></div>
                    <div class="power-label">Voltage: <span id="voltage2">0.00 V</span></div>
                </div>
                <div class="power-item">
                    <div class="power-label">Channel 3</div>
                    <div class="power-value" id="power3">0.00 W</div>
                    <div class="power-label">Current: <span id="current3">0.00 mA</span></div>
                    <div class="power-label">Voltage: <span id="voltage3">0.00 V</span></div>
                </div>
            </div>
        </div>

        <a href="/config" class="config-link">
            <i class="fas fa-cog"></i> WiFi Configuration
        </a>

        <a href="#" onclick="showTimerConfig(0)" class="config-link">
            <i class="fas fa-clock"></i> Timer Configuration
        </a>

        <a href="#" onclick="showCurrentProtection()" class="config-link">
            <i class="fas fa-bolt"></i> Current Protection
        </a>

        <div class="footer">
            <div class="build-date">Build: <span id="buildDate">Loading...</span></div>
            <a href="/upgrade" class="upgrade-link">
                <i class="fas fa-upload"></i> Upgrade
            </a>
        </div>
    </div>

    <!-- 添加模态框 -->
    <div id="powerChartModal" class="modal">
        <div class="modal-content">
            <span class="close">&times;</span>
            <h3 id="modalTitle">Power Monitor Chart</h3>
            <div class="chart-controls">
                <select id="dataTypeSelect" onchange="updateChartType()">
                    <option value="power">Power (W)</option>
                    <option value="current">Current (mA)</option>
                    <option value="voltage">Voltage (V)</option>
                </select>
            </div>
            <div class="chart-container">
                <canvas id="powerChart"></canvas>
            </div>
        </div>
    </div>

    <!-- 添加定时器配置模态框 -->
    <div id="timerModal" class="modal">
        <div class="modal-content">
            <span class="close">&times;</span>
            <h3>Timer Configuration</h3>
            <div class="timer-config">
                <div class="timer-section">
                    <h4>Timer Settings</h4>
                    <div class="timer-inputs">
                        <div class="input-group">
                            <label>Relay:</label>
                            <select id="timerRelay">
                                <option value="0">Relay 1</option>
                                <option value="1">Relay 2</option>
                                <option value="2">Relay 3</option>
                            </select>
                        </div>
                        <div class="input-group">
                            <label>Time:</label>
                            <input type="time" id="timerTime">
                        </div>
                        <div class="input-group">
                            <label>Action:</label>
                            <select id="timerAction">
                                <option value="true">Turn ON</option>
                                <option value="false">Turn OFF</option>
                            </select>
                        </div>
                        <div class="input-group">
                            <label>Repeat:</label>
                            <select id="timerRepeat" onchange="toggleWeekdaySelection()">
                                <option value="0">Once</option>
                                <option value="1">Daily</option>
                                <option value="2">Weekdays</option>
                                <option value="3">Weekends</option>
                                <option value="4">Custom</option>
                            </select>
                        </div>
                        <div id="weekdaySelection" style="display: none;">
                            <label>Weekdays:</label>
                            <div class="weekday-buttons">
                                <label><input type="checkbox" value="0"> Sun</label>
                                <label><input type="checkbox" value="1"> Mon</label>
                                <label><input type="checkbox" value="2"> Tue</label>
                                <label><input type="checkbox" value="3"> Wed</label>
                                <label><input type="checkbox" value="4"> Thu</label>
                                <label><input type="checkbox" value="5"> Fri</label>
                                <label><input type="checkbox" value="6"> Sat</label>
                            </div>
                        </div>
                    </div>
                    <div class="timer-actions">
                        <button onclick="saveTimer()" class="timer-btn">Save Timer</button>
                    </div>
                </div>
                <div class="timer-list">
                    <h4>Active Timers</h4>
                    <div id="timerList"></div>
                </div>
            </div>
        </div>
    </div>

    <!-- 添加电流保护配置模态框 -->
    <div id="currentProtectionModal" class="modal">
        <div class="modal-content">
            <span class="close">&times;</span>
            <h3>Current Protection Settings</h3>
            <div class="protection-config">
                <div class="protection-section">
                    <h4>Channel Settings</h4>
                    <div class="protection-inputs">
                        <div class="input-group">
                            <label>Channel 1 Current Limit (mA):</label>
                            <input type="number" id="currentLimit1" min="0" max="10000" step="100">
                        </div>
                        <div class="input-group">
                            <label>Channel 2 Current Limit (mA):</label>
                            <input type="number" id="currentLimit2" min="0" max="10000" step="100">
                        </div>
                        <div class="input-group">
                            <label>Channel 3 Current Limit (mA):</label>
                            <input type="number" id="currentLimit3" min="0" max="10000" step="100">
                        </div>
                    </div>
                    <div class="protection-actions">
                        <button onclick="saveCurrentProtection()" class="timer-btn">Save Settings</button>
                    </div>
                </div>
                <div class="protection-status">
                    <h4>Protection Status</h4>
                    <div id="protectionStatus"></div>
                </div>
            </div>
        </div>
    </div>

    <script>
        let ws;
        let powerChart;
        let chartData = {
            labels: [],
            current: [],
            voltage: [],
            power: []
        };
        const maxDataPoints = 60;
        let currentChannelIndex = 0;
        let currentDataType = 'power';
        let currentTimerId = null;
        let currentRelay = 0;

        // 定期检查继电器状态
        function checkRelayStates() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    if (data.relays) {
                        for (let i = 0; i < data.relays.length; i++) {
                            const state = data.relays[i];
                            document.getElementById('status' + (i + 1)).textContent = state ? 'ON' : 'OFF';
                            document.querySelector(`input[type="checkbox"][onchange="toggleRelay(${i + 1}, this.checked)"]`).checked = state;
                        }
                    }
                })
                .catch(error => console.error('Error checking relay states:', error));
        }

        function toggleRelay(relay, state) {
            fetch('/relay/' + relay + '?state=' + (state ? '1' : '0'))
                .then(response => {
                    if (!response.ok) {
                        throw new Error('Network response was not ok');
                    }
                    return response.json();
                })
                .then(data => {
                    // 更新状态显示
                    document.getElementById('status' + relay).textContent = state ? 'ON' : 'OFF';
                })
                .catch(error => {
                    console.error('Error toggling relay:', error);
                    // 如果发生错误，重新检查状态以确保显示正确
                    checkRelayStates();
                });
        }

        function updatePowerData() {
            fetch('/power')
                .then(response => response.json())
                .then(data => {
                    for (let i = 1; i <= 3; i++) {
                        const channel = data['channel' + i];
                        if (channel) {
                            document.getElementById('power' + i).textContent = channel.power.toFixed(2) + ' W';
                            document.getElementById('current' + i).textContent = channel.current.toFixed(2) + ' mA';
                            document.getElementById('voltage' + i).textContent = channel.voltage.toFixed(2) + ' V';
                            
                            // 如果图表正在显示，更新图表数据
                            if (powerChart && currentChannelIndex === i - 1) {
                                updateChartData(channel);
                            }
                        }
                    }
                })
                .catch(error => console.error('Error updating power data:', error));
        }

        // 显示功率图表
        function showPowerChart(channelIndex) {
            currentChannelIndex = channelIndex;
            const modal = document.getElementById('powerChartModal');
            const modalTitle = document.getElementById('modalTitle');
            modalTitle.textContent = `Channel ${channelIndex + 1} Power Monitor`;
            
            // 初始化图表
            if (!powerChart) {
                const ctx = document.getElementById('powerChart').getContext('2d');
                powerChart = new Chart(ctx, {
                    type: 'line',
                    data: {
                        labels: [],
                        datasets: [{
                            label: 'Power (W)',
                            data: [],
                            borderColor: 'rgb(54, 162, 235)',
                            tension: 0.1
                        }]
                    },
                    options: {
                        responsive: true,
                        maintainAspectRatio: false,
                        scales: {
                            y: {
                                beginAtZero: true
                            }
                        }
                    }
                });
            }

            // 清空现有数据
            chartData.labels = [];
            chartData.current = [];
            chartData.voltage = [];
            chartData.power = [];
            
            // 显示模态框
            modal.style.display = 'block';
        }

        function updateChartType() {
            const select = document.getElementById('dataTypeSelect');
            currentDataType = select.value;
            
            if (powerChart) {
                const labels = {
                    'power': 'Power (W)',
                    'current': 'Current (mA)',
                    'voltage': 'Voltage (V)'
                };
                
                powerChart.data.datasets[0].label = labels[currentDataType];
                powerChart.data.datasets[0].data = chartData[currentDataType];
                powerChart.update();
            }
        }

        function updateChartData(data) {
            if (!powerChart) return;

            const timestamp = new Date().toLocaleTimeString();
            
            // 添加新数据
            chartData.labels.push(timestamp);
            chartData.current.push(data.current);
            chartData.voltage.push(data.voltage);
            chartData.power.push(data.power);

            // 保持最近60个数据点
            if (chartData.labels.length > maxDataPoints) {
                chartData.labels.shift();
                chartData.current.shift();
                chartData.voltage.shift();
                chartData.power.shift();
            }

            // 更新图表
            powerChart.data.labels = chartData.labels;
            powerChart.data.datasets[0].data = chartData[currentDataType];
            powerChart.update();
        }

        // 关闭模态框
        document.querySelectorAll('.close').forEach(closeBtn => {
            closeBtn.onclick = function() {
                document.getElementById('powerChartModal').style.display = 'none';
                document.getElementById('timerModal').style.display = 'none';
                document.getElementById('currentProtectionModal').style.display = 'none';
            }
        });

        // 点击模态框外部关闭
        window.onclick = function(event) {
            const powerModal = document.getElementById('powerChartModal');
            const timerModal = document.getElementById('timerModal');
            const protectionModal = document.getElementById('currentProtectionModal');
            if (event.target == powerModal) {
                powerModal.style.display = 'none';
            }
            if (event.target == timerModal) {
                timerModal.style.display = 'none';
            }
            if (event.target == protectionModal) {
                protectionModal.style.display = 'none';
            }
        }

        // 为每个功率监测项添加点击事件
        document.addEventListener('DOMContentLoaded', function() {
            const powerItems = document.querySelectorAll('.power-monitor .power-item');
            powerItems.forEach((item, index) => {
                item.classList.add('monitor');
                item.addEventListener('click', () => showPowerChart(index));
            });

            // 为每个继电器的Timer按钮添加点击事件
            for (let i = 1; i <= 3; i++) {
                const timerBtn = document.querySelector(`button[onclick="showTimerConfig(${i})"]`);
                if (timerBtn) {
                    timerBtn.addEventListener('click', () => showTimerConfig(i));
                }
            }

            // 初始更新
            checkRelayStates();
            updatePowerData();
            updateBuildDate();
        });

        // 定期更新数据
        setInterval(checkRelayStates, 1000);  // 每秒检查继电器状态
        setInterval(updatePowerData, 1000);   // 每秒更新电源数据

        // 更新构建日期
        function updateBuildDate() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    if (data.build_date) {
                        document.getElementById('buildDate').textContent = data.build_date;
                    }
                })
                .catch(error => console.error('Error updating build date:', error));
        }

        function showTimerConfig(relay) {
            currentRelay = relay;
            currentTimerId = null;  // 重置当前定时器ID
            document.getElementById('timerTime').value = '';
            document.getElementById('timerAction').value = 'true';
            document.getElementById('timerRepeat').value = '1';
            document.getElementById('timerRelay').value = relay;
            toggleWeekdaySelection();
            loadTimers();
            document.getElementById('timerModal').style.display = 'block';
        }
        
        function loadTimers() {
            fetch('/timer')
                .then(response => response.json())
                .then(data => {
                    const timerList = document.getElementById('timerList');
                    timerList.innerHTML = '';
                    
                    if (data.timers) {
                        data.timers.forEach(timer => {
                            const timerItem = document.createElement('div');
                            timerItem.className = 'timer-item';
                            timerItem.innerHTML = `
                                <div class="timer-info">
                                    Relay ${timer.relayId + 1} - 
                                    ${formatTime(timer.hour, timer.minute)} - 
                                    ${timer.state ? 'ON' : 'OFF'} - 
                                    ${formatRepeat(timer.repeat)}
                                </div>
                                <div class="timer-actions">
                                    <button onclick="editTimer(${timer.id})" class="timer-btn">
                                        <i class="fas fa-edit"></i>
                                    </button>
                                    <button onclick="deleteTimer(${timer.id})" class="timer-btn delete">
                                        <i class="fas fa-trash"></i>
                                    </button>
                                </div>
                            `;
                            timerList.appendChild(timerItem);
                        });
                    }
                })
                .catch(error => {
                    console.error('Error loading timers:', error);
                    alert('Failed to load timers. Please try again.');
                });
        }
        
        function formatTime(hour, minute) {
            return `${String(hour).padStart(2, '0')}:${String(minute).padStart(2, '0')}`;
        }
        
        function formatRepeat(repeat) {
            switch (parseInt(repeat)) {
                case 0: return 'Once';
                case 1: return 'Daily';
                case 2: return 'Weekdays';
                case 3: return 'Weekends';
                case 4: return 'Custom';
                default: return 'Unknown';
            }
        }
        
        function editTimer(id) {
            currentTimerId = id;
            fetch(`/timer?id=${id}`)
                .then(response => response.json())
                .then(timer => {
                    document.getElementById('timerTime').value = 
                        `${String(timer.hour).padStart(2, '0')}:${String(timer.minute).padStart(2, '0')}`;
                    document.getElementById('timerAction').value = timer.state.toString();
                    document.getElementById('timerRepeat').value = timer.repeat.toString();
                    document.getElementById('timerRelay').value = timer.relayId;
                    
                    if (timer.repeat === 4) {
                        const weekdays = document.querySelectorAll('#weekdaySelection input[type="checkbox"]');
                        weekdays.forEach(checkbox => {
                            checkbox.checked = (timer.weekdays & (1 << parseInt(checkbox.value))) !== 0;
                        });
                    }
                    
                    toggleWeekdaySelection();
                })
                .catch(error => {
                    console.error('Error loading timer:', error);
                    alert('Failed to load timer. Please try again.');
                });
        }
        
        function toggleWeekdaySelection() {
            const repeat = document.getElementById('timerRepeat').value;
            document.getElementById('weekdaySelection').style.display = 
                repeat === '4' ? 'block' : 'none';
        }
        
        function saveTimer() {
            const time = document.getElementById('timerTime').value.split(':');
            if (!time[0] || !time[1]) {
                alert('Please enter a valid time');
                return;
            }

            const hour = parseInt(time[0]);
            const minute = parseInt(time[1]);
            const state = document.getElementById('timerAction').value === 'true';
            const repeat = parseInt(document.getElementById('timerRepeat').value);
            const relayId = parseInt(document.getElementById('timerRelay').value);
            
            let weekdays = 0;
            if (repeat === 4) {
                document.querySelectorAll('#weekdaySelection input[type="checkbox"]:checked')
                    .forEach(checkbox => {
                        weekdays |= (1 << parseInt(checkbox.value));
                    });
            }
            
            const timer = {
                id: currentTimerId || Date.now(),
                relayId: relayId,
                hour: hour,
                minute: minute,
                enabled: true,
                state: state,
                repeat: repeat,
                weekdays: weekdays
            };
            
            const method = currentTimerId ? 'PUT' : 'POST';
            
            fetch('/timer', {
                method: method,
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(timer)
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    loadTimers();
                    // 重置表单
                    currentTimerId = null;
                    document.getElementById('timerTime').value = '';
                    document.getElementById('timerAction').value = 'true';
                    document.getElementById('timerRepeat').value = '1';
                    document.getElementById('timerRelay').value = currentRelay;
                    toggleWeekdaySelection();
                } else {
                    alert('Failed to save timer: ' + (data.error || 'Unknown error'));
                }
            })
            .catch(error => {
                console.error('Error saving timer:', error);
                alert('Failed to save timer. Please try again.');
            });
        }
        
        function deleteTimer(id) {
            if (confirm('Are you sure you want to delete this timer?')) {
                fetch('/timer?id=' + id, {
                    method: 'DELETE'
                })
                .then(response => response.json())
                .then(data => {
                    if (data.success) {
                        loadTimers();
                    } else {
                        alert('Failed to delete timer: ' + (data.error || 'Unknown error'));
                    }
                })
                .catch(error => {
                    console.error('Error deleting timer:', error);
                    alert('Failed to delete timer. Please try again.');
                });
            }
        }

        function showCurrentProtection() {
            // 加载当前设置
            fetch('/protection')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('currentLimit1').value = data.channel1 || 0;
                    document.getElementById('currentLimit2').value = data.channel2 || 0;
                    document.getElementById('currentLimit3').value = data.channel3 || 0;
                    updateProtectionStatus();
                })
                .catch(error => {
                    console.error('Error loading protection settings:', error);
                    alert('Failed to load protection settings');
                });
            
            document.getElementById('currentProtectionModal').style.display = 'block';
        }

        function saveCurrentProtection() {
            const settings = {
                channel1: parseInt(document.getElementById('currentLimit1').value) || 0,
                channel2: parseInt(document.getElementById('currentLimit2').value) || 0,
                channel3: parseInt(document.getElementById('currentLimit3').value) || 0
            };

            fetch('/protection', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(settings)
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    alert('Settings saved successfully');
                    updateProtectionStatus();
                } else {
                    alert('Failed to save settings: ' + (data.error || 'Unknown error'));
                }
            })
            .catch(error => {
                console.error('Error saving protection settings:', error);
                alert('Failed to save settings');
            });
        }

        function updateProtectionStatus() {
            fetch('/protection/status')
                .then(response => response.json())
                .then(data => {
                    const statusDiv = document.getElementById('protectionStatus');
                    let html = '';
                    for (let i = 1; i <= 3; i++) {
                        const channel = data[`channel${i}`];
                        html += `
                            <div class="protection-item">
                                <div class="protection-label">Channel ${i}:</div>
                                <div class="protection-value">
                                    Current: ${channel.current.toFixed(2)} mA
                                    ${channel.triggered ? '<span class="warning">(Protection Triggered)</span>' : ''}
                                </div>
                            </div>
                        `;
                    }
                    statusDiv.innerHTML = html;
                })
                .catch(error => {
                    console.error('Error updating protection status:', error);
                });
        }

        // 定期更新保护状态
        setInterval(updateProtectionStatus, 1000);
    </script>
</body>
</html>
)rawliteral";

WebServer::WebServer(EspSmartWifi& wifi, EasyLed& led, Display& display) 
    : server(80), wifi(wifi), led(led), display(display) {
    Serial.println("\n=== WebServer Initialization ===");
    
    // 初始化SPIFFS
    Serial.println("Initializing SPIFFS...");
    if (!SPIFFS.begin()) {
        Serial.println("ERROR: SPIFFS initialization failed!");
        return;
    }
    Serial.println("SUCCESS: SPIFFS initialized successfully");
    
    // 检查文件系统状态
    FSInfo fs_info;
    SPIFFS.info(fs_info);
    Serial.print("Total bytes: ");
    Serial.println(fs_info.totalBytes);
    Serial.print("Used bytes: ");
    Serial.println(fs_info.usedBytes);
    
    // 初始化继电器引脚
    Serial.println("\nInitializing relay pins...");
    for(int i = 0; i < 3; i++) {
        pinMode(RELAY_PINS[i], OUTPUT);
        digitalWrite(RELAY_PINS[i], LOW);  // 默认关闭（HIGH为关闭，LOW为开启）
        Serial.print("Pin D");
        Serial.print(RELAY_PINS[i] == RELAY_PIN_1 ? "1" : 
                    RELAY_PINS[i] == RELAY_PIN_2 ? "2" : 
                    RELAY_PINS[i] == RELAY_PIN_3 ? "3" : "5");
        Serial.println(" initialized");
    }
    
    // 初始化按钮引脚
    Serial.println("\nInitializing button pin...");
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    Serial.println("Button pin initialized with internal pull-up");
    
    // 初始化PowerMonitor
    Serial.println("\nInitializing PowerMonitor...");
    if (initPowerMonitor()) {
        Serial.println("SUCCESS: PowerMonitor initialized successfully");
    } else {
        Serial.println("ERROR: PowerMonitor initialization failed!");
    }
    
    // 初始化定时器配置
    Serial.println("\nInitializing timer configuration...");
    timerCount = 0;  // 重置定时器计数
    
    // 加载保存的状态
    Serial.println("\nLoading saved relay states...");
    bool states[4];
    if (wifi.getRelayStates(states)) {  // 使用公共方法 getRelayStates
        Serial.println("SUCCESS: Relay states loaded from config");
        Serial.println("Applying loaded states to hardware...");
        for(int i = 0; i < 3; i++) {
            digitalWrite(RELAY_PINS[i], states[i] ? HIGH:LOW);
            Serial.print("Relay ");
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.println(states[i] ? "OFF" : "ON");
        }
        
        // 初始化显示缓存
        display.updateRelayStates(states);
    } else {
        Serial.println("WARNING: No saved states found or error loading states");
        Serial.println("Using default states (all OFF)");
    }

    // 加载电流保护配置
    Serial.println("\nLoading current protection settings...");
    loadProtectionConfig();
    Serial.println("Current protection settings loaded");
    
    Serial.println("=== WebServer Initialization Complete ===\n");
}

bool WebServer::initPowerMonitor() {
    return powerMonitor.begin();
}

void WebServer::begin() {
    // 先停止服务器
    server.stop();
    
    // 重新初始化路由
    if (wifi.isAPMode())
    {
        server.on("/", HTTP_GET, [this]() { HandleConfigRoot(); });
        server.on("/config/data", HTTP_POST, [this]() { HandleConfigSave(); });
        server.on("/config/data", HTTP_GET, [this]() { handleGetConfigData(); });  // 添加新的配置数据接口
    }
    else
    {
        server.on("/", HTTP_GET, [this]() { handleRoot(); });
        server.on("/config", HTTP_GET, [this]() { HandleConfigRoot(); });
        server.on("/config/data", HTTP_POST, [this]() { HandleConfigSave(); });
        server.on("/config/data", HTTP_GET, [this]() { handleGetConfigData(); });  // 添加新的配置数据接口
    }
    server.on("/status", HTTP_GET, [this]() { handleStatus(); });
    server.on("/relay/1", HTTP_GET, [this]() { handleRelay(1); });
    server.on("/relay/2", HTTP_GET, [this]() { handleRelay(2); });
    server.on("/relay/3", HTTP_GET, [this]() { handleRelay(3); });
    server.on("/power", HTTP_GET, [this]() { handlePower(); });
    server.on("/restart", HTTP_POST, [this]() { handleRestart(); });
    server.on("/upgrade", HTTP_GET, [this]() { handleUpgrade(); });
    server.on("/update", HTTP_POST, [this]() { handleUpdate(); }, [this]() { handleUpdateUpload(); });
    
    // 定时器相关路由
    server.on("/timer", HTTP_GET, [this]() { handleGetTimerConfig(); });
    server.on("/timer", HTTP_POST, [this]() { handleAddTimer(); });
    server.on("/timer", HTTP_PUT, [this]() { handleTimerConfig(); });
    server.on("/timer", HTTP_DELETE, [this]() { handleDeleteTimer(); });
    
    // 电流保护相关路由
    server.on("/protection", HTTP_GET, [this]() { handleGetProtection(); });
    server.on("/protection", HTTP_POST, [this]() { handleSetProtection(); });
    server.on("/protection/status", HTTP_GET, [this]() { handleProtectionStatus(); });
    
    server.onNotFound([this]() { handleNotFound(); });
    
    // 启动服务器
    server.begin();
    Serial.println("HTTP server started");
    
    // 初始化时间
    initTime();
    // 加载定时器配置
    loadTimerConfig();
}

void WebServer::handleClient() {
    server.handleClient();
    

    // 检查定时器
    if (millis() - lastTimeCheck >= timeCheckInterval) {
        checkTimers();
        lastTimeCheck = millis();
    }
    
    // 检查电流保护
    checkCurrentProtection();
}

void WebServer::stop() {
    server.stop();
    Serial.println("HTTP server stopped");
}

void WebServer::updateRelay(int index, bool state) {
    if(index >= 0 && index < 3) {
        // 获取当前缓存状态
        bool states[4];
        if (!wifi.getRelayStates(states)) {
            Serial.println("Failed to get relay states");
            return;
        }
        
        // 只有当状态发生变化时才更新
        if (states[index] != state) {
            // 更新硬件状态
            digitalWrite(RELAY_PINS[index], state ? HIGH:LOW);
            
            // 添加串口日志
            Serial.print("Relay ");
            Serial.print(index + 1);
            Serial.print(" changed to ");
            Serial.println(state ? "ON" : "OFF");
            
            // 打印详细信息
            Serial.print("Pin: D");
            Serial.print(RELAY_PINS[index] == D5 ? "1" : 
                        RELAY_PINS[index] == D6 ? "2" : 
                        RELAY_PINS[index] == D7 ? "3" : "5");
            Serial.print(", State: ");
            Serial.println(state ? "HIGH (ON)" : "LOW (OFF)");
            
            // 保存状态到配置
            if (wifi.updateRelayState(index, state)) {
                Serial.println("Relay state saved successfully");
                
                // 更新显示缓存
                states[index] = state;
                display.updateRelayStates(states);

                // 重置保护状态
                protectionTriggered[index] = false;
            } else {
                Serial.println("Failed to save relay state");
            }

            // 广播状态更新到所有连接的客户端
            StaticJsonDocument<128> doc;
            doc["relay"] = index + 1;
            doc["state"] = state;
            String response;
            serializeJson(doc, response);
            server.send(200, "application/json", response);
        }
    }
}

void WebServer::handleRelay(int relay) {
    if (!server.hasArg("state")) {
        server.send(400, "text/plain", "Missing state parameter");
        return;
    }

    bool state = server.arg("state") == "1";
    updateRelay(relay - 1, state);  // 转换为0-based索引
    
    StaticJsonDocument<128> doc;
    doc["success"] = true;
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void WebServer::handlePower() {
    StaticJsonDocument<512> doc;
    
    if (powerMonitor.isInitialized()) {
        for (int i = 0; i < 3; i++) {
            JsonObject channel = doc.createNestedObject("channel" + String(i + 1));
            channel["current"] = powerMonitor.getCurrent_mA(i);
            channel["voltage"] = powerMonitor.getBusVoltage_V(i);
            channel["power"] = powerMonitor.getPower_mW(i) / 1000.0;  // 转换为瓦特
        }
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void WebServer::handleRoot() {
    Serial.println("Handling root request");
    
    // 检查 INDEX_HTML 是否有效
    if (strlen(INDEX_HTML) == 0) {
        Serial.println("ERROR: INDEX_HTML is empty!");
        server.send(500, "text/plain", "Internal Server Error");
        return;
    }
    
    // 发送 HTML 内容
    server.send(200, "text/html", INDEX_HTML);  // 直接使用 INDEX_HTML，不使用 FPSTR
    Serial.println("HTML content sent successfully");
}

void WebServer::handleStatus() {
    StaticJsonDocument<512> doc;
    
    // WiFi status
    doc["wifi"]["connected"] = WiFi.status() == WL_CONNECTED;
    doc["wifi"]["ssid"] = WiFi.SSID();
    doc["wifi"]["rssi"] = WiFi.RSSI();
    doc["wifi"]["ip"] = WiFi.localIP().toString();
    
    // Relay states
    bool states[3];
    if (wifi.getRelayStates(states)) {
        JsonArray statesArray = doc.createNestedArray("relays");
        for (int i = 0; i < 3; i++) {
            statesArray.add(states[i]);
        }
    }

    // Build date
    doc["build_date"] = __DATE__ " " __TIME__;
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}


void WebServer::handleRestart() {
    server.send(200, "text/plain", "Restarting...");
    delay(1000);
    ESP.restart();
}


void WebServer::HandleConfigRoot()
{
    String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>ESP8266 WiFi Configuration</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background: #f0f0f0; }
        .container { max-width: 400px; margin: 0 auto; background: white; padding: 20px; border-radius: 5px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
        h1 { text-align: center; color: #333; }
        .form-group { margin-bottom: 15px; }
        label { display: block; margin-bottom: 5px; color: #666; }
        input[type="text"], input[type="password"] { width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; }
        button { width: 100%; padding: 10px; background: #4CAF50; color: white; border: none; border-radius: 4px; cursor: pointer; }
        button:hover { background: #45a049; }
        .status { margin-top: 20px; padding: 10px; border-radius: 4px; }
        .success { background: #dff0d8; color: #3c763d; }
        .error { background: #f2dede; color: #a94442; }
        .help-text { font-size: 12px; color: #666; margin-top: 5px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>WiFi Configuration</h1>
        <form action="/config/data" method="post">
            <div class="form-group">
                <label for="ssid">WiFi SSID:</label>
                <input type="text" id="ssid" name="ssid" required>
            </div>
            <div class="form-group">
                <label for="passwd">WiFi Password:</label>
                <input type="password" id="passwd" name="passwd" required>
            </div>
            <div class="form-group">
                <label for="server">MQTT Server:</label>
                <input type="text" id="server" name="server" placeholder="mqtt://username:passwd@mqtt.server" required>
                <div class="help-text">Format: mqtt://username:passwd@mqtt.server</div>
            </div>
            <div class="form-group">
                <label for="topic">MQTT Topic:</label>
                <input type="text" id="topic" name="topic" placeholder="/espRouterPower/power" required>
                <div class="help-text">MQTT topic for power data publishing</div>
            </div>
            <button type="submit">Save Configuration</button>
        </form>
    </div>
    <script>
        // 页面加载完成后获取配置
        window.onload = function() {
            fetch('/config/data')
                .then(response => response.json())
                .then(config => {
                    document.getElementById('ssid').value = config.ssid || '';
                    document.getElementById('passwd').value = config.passwd || '';
                    document.getElementById('server').value = config.server || '';
                    document.getElementById('topic').value = config.topic || '';
                })
                .catch(error => console.error('Error loading config:', error));
        };
    </script>
</body>
</html>
)";
    server.send(200, "text/html", html);
}

void WebServer::HandleConfigSave()
{
    if (!server.hasArg("ssid") || !server.hasArg("passwd") || 
        !server.hasArg("server") || !server.hasArg("topic")) {
        server.send(400, "text/plain", "Missing parameters");
        return;
    }
    Config config;
    config.SSID = server.arg("ssid");
    config.Passwd = server.arg("passwd");
    config.Server = server.arg("server");
    config.Topic = server.arg("topic");
    
    if (wifi.SaveConfig(config)) {
        String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Configuration Saved</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background: #f0f0f0; }
        .container { max-width: 400px; margin: 0 auto; background: white; padding: 20px; border-radius: 5px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
        h1 { text-align: center; color: #333; }
        .message { text-align: center; margin: 20px 0; }
        .restart { text-align: center; }
        .restart a { display: inline-block; padding: 10px 20px; background: #4CAF50; color: white; text-decoration: none; border-radius: 4px; }
        .restart a:hover { background: #45a049; }
    </style>
    <script>
        setTimeout(function() {
            window.location.href = '/';
        }, 5000);
    </script>
</head>
<body>
    <div class="container">
        <h1>Configuration Saved</h1>
        <div class="message">
            <p>WiFi configuration has been saved successfully.</p>
            <p>The device will now attempt to connect to the configured WiFi network.</p>
            <p>This page will refresh in 5 seconds...</p>
        </div>
    </div>
</body>
</html>
)";
        server.send(200, "text/html", html);
        
        // 延迟重启，让页面有时间显示
        delay(1000);
        ESP.restart();
    } else {
        server.send(500, "text/plain", "Failed to save configuration");
    }
}


void WebServer::handleUpgrade() {
    String html = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <title>Firmware Upgrade</title>
            <meta name="viewport" content="width=device-width, initial-scale=1">
            <style>
                body {
                    font-family: Arial, sans-serif;
                    max-width: 800px;
                    margin: 0 auto;
                    padding: 20px;
                }
                .upload-form {
                    background: #f8f9fa;
                    padding: 20px;
                    border-radius: 8px;
                    margin-top: 20px;
                }
                .btn {
                    background: #2196F3;
                    color: white;
                    padding: 10px 20px;
                    border: none;
                    border-radius: 4px;
                    cursor: pointer;
                }
                .btn:hover {
                    background: #1976D2;
                }
                .progress {
                    margin-top: 20px;
                    display: none;
                }
                .progress-bar {
                    width: 100%;
                    height: 20px;
                    background: #eee;
                    border-radius: 10px;
                    overflow: hidden;
                }
                .progress-bar-fill {
                    height: 100%;
                    background: #4CAF50;
                    width: 0%;
                    transition: width 0.3s ease;
                }
            </style>
        </head>
        <body>
            <h1>Firmware Upgrade</h1>
            <div class="upload-form">
                <form method="POST" action="/update" enctype="multipart/form-data">
                    <input type="file" name="firmware" accept=".bin">
                    <button type="submit" class="btn">Upload Firmware</button>
                </form>
            </div>
            <div class="progress">
                <div class="progress-bar">
                    <div class="progress-bar-fill"></div>
                </div>
                <div id="status"></div>
            </div>
            <script>
                document.querySelector('form').onsubmit = function() {
                    document.querySelector('.progress').style.display = 'block';
                    var xhr = new XMLHttpRequest();
                    var formData = new FormData(this);
                    
                    xhr.upload.addEventListener('progress', function(e) {
                        if (e.lengthComputable) {
                            var percent = (e.loaded / e.total) * 100;
                            document.querySelector('.progress-bar-fill').style.width = percent + '%';
                        }
                    });
                    
                    xhr.onreadystatechange = function() {
                        if (xhr.readyState === 4) {
                            if (xhr.status === 200) {
                                document.getElementById('status').textContent = 'Upload complete! Rebooting...';
                                setTimeout(function() {
                                    window.location.href = '/';
                                }, 5000);
                            } else {
                                document.getElementById('status').textContent = 'Upload failed: ' + xhr.statusText;
                            }
                        }
                    };
                    
                    xhr.open('POST', '/update', true);
                    xhr.send(formData);
                    return false;
                };
            </script>
        </body>
        </html>
    )";
    server.send(200, "text/html", html);
}

void WebServer::handleUpdate() {
    server.send(200, "text/plain", "Update complete. Rebooting...");
    delay(1000);
    ESP.restart();
}

void WebServer::handleUpdateUpload() {
    HTTPUpload& upload = server.upload();
    
    if (upload.status == UPLOAD_FILE_START) {
        Serial.println("Update: " + upload.filename);
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace)) {
            Serial.println("Update begin failed");
            server.send(500, "text/plain", "Update begin failed");
            return;
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Serial.println("Update write failed");
            server.send(500, "text/plain", "Update write failed");
            return;
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
            Serial.println("Update complete");
        } else {
            Serial.println("Update failed");
            server.send(500, "text/plain", "Update failed");
        }
    }
}

void WebServer::handleNotFound() {
    Serial.print("404 Not Found: ");
    Serial.println(server.uri());
    server.send(404, "text/plain", "Not found");
}

void WebServer::handleButtonPress() {
    // 获取当前继电器状态
    bool states[4];
    if (wifi.getRelayStates(states)) {
        // 检查是否有任何继电器是开启的
        bool anyOn = false;
        for (int i = 0; i < 3; i++) {
            if (states[i]) {
                anyOn = true;
                break;
            }
        }
        
        // 如果任何继电器是开启的，关闭所有继电器
        // 否则，打开所有继电器
        bool newState = !anyOn;
        for (int i = 0; i < 3; i++) {
            updateRelay(i, newState);
        }
        
        Serial.print("Button pressed - All relays turned ");
        Serial.println(newState ? "ON" : "OFF");

        // 广播状态更新到所有连接的客户端
        StaticJsonDocument<512> doc;
        JsonArray statesArray = doc.createNestedArray("states");
        for (int i = 0; i < 3; i++) {
            statesArray.add(newState);
        }
        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response);
    }
}

void WebServer::initTime() {
    configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("Waiting for NTP time sync...");
    delay(1000);
    while (!time(nullptr)) {
        Serial.print(".");
        delay(100);
    }
    Serial.println("\nTime synchronized");
}

void WebServer::checkTimers() {
    time_t now = time(nullptr);
    if (now == 0) {
        syncTime();  // 如果时间无效，尝试重新同步
        return;
    }
    
    struct tm* timeinfo = localtime(&now);
    if (!timeinfo) return;
    
    // 获取当前继电器状态
    bool states[4];
    if (!wifi.getRelayStates(states)) {
        Serial.println("Failed to get relay states");
        return;
    }
    
    // 检查所有定时器
    for (int i = 0; i < timerCount; i++) {
        if (shouldTriggerTimer(timers[i], timeinfo)) {
            // 使用缓存的状态进行比较
            if (states[timers[i].relayId] != timers[i].state) {
                // 更新继电器状态
                updateRelay(timers[i].relayId, timers[i].state);
                
                // 更新最后触发时间
                timers[i].lastTriggered = now;
                
                // 如果是一次性定时器，禁用它
                if (timers[i].repeat == TimerRepeat::ONCE) {
                    timers[i].enabled = false;
                }
                
                // 保存配置
                saveTimerConfig();
                
                Serial.print("Timer ");
                Serial.print(timers[i].id);
                Serial.print(" triggered: Relay ");
                Serial.print(timers[i].relayId + 1);
                Serial.print(" turned ");
                Serial.println(timers[i].state ? "ON" : "OFF");
            }
        }
    }
}

void WebServer::handleGetTimerConfig() {
    // 检查是否有路径参数
    if (server.hasArg("id")) {
        // 从查询参数获取ID
        String idStr = server.arg("id");
        uint32_t id = idStr.toInt();
        
        Timer* timer = getTimer(id);
        if (!timer) {
            server.send(404, "application/json", "{\"error\":\"Timer not found\"}");
            return;
        }
        
        StaticJsonDocument<256> doc;  // 减小文档大小
        doc["id"] = timer->id;
        doc["relayId"] = timer->relayId;
        doc["hour"] = timer->hour;
        doc["minute"] = timer->minute;
        doc["enabled"] = timer->enabled;
        doc["state"] = timer->state;
        doc["repeat"] = static_cast<int>(timer->repeat);
        doc["weekdays"] = timer->weekdays;
        doc["lastTriggered"] = timer->lastTriggered;
        
        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response);
    } else {
        // 获取所有定时器
        StaticJsonDocument<1024> doc;  // 减小文档大小
        JsonArray timersArray = doc.createNestedArray("timers");
        
        for (int i = 0; i < timerCount; i++) {
            JsonObject timerObj = timersArray.createNestedObject();
            timerObj["id"] = timers[i].id;
            timerObj["relayId"] = timers[i].relayId;
            timerObj["hour"] = timers[i].hour;
            timerObj["minute"] = timers[i].minute;
            timerObj["enabled"] = timers[i].enabled;
            timerObj["state"] = timers[i].state;
            timerObj["repeat"] = static_cast<int>(timers[i].repeat);
            timerObj["weekdays"] = timers[i].weekdays;
            timerObj["lastTriggered"] = timers[i].lastTriggered;
        }
        
        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response);
    }
}

void WebServer::handleAddTimer() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Missing request body\"}");
        return;
    }
    
    String data = server.arg("plain");
    StaticJsonDocument<256> doc;  // 减小文档大小
    DeserializationError error = deserializeJson(doc, data);
    
    if (error) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }
    
    Timer timer;
    timer.id = doc["id"] | random(0xFFFFFFFF);
    timer.relayId = doc["relayId"] | 0;
    timer.hour = doc["hour"] | 0;
    timer.minute = doc["minute"] | 0;
    timer.enabled = doc["enabled"] | true;
    timer.state = doc["state"] | false;
    timer.repeat = static_cast<TimerRepeat>(doc["repeat"].as<int>() | 0);
    timer.weekdays = doc["weekdays"] | 0;
    timer.lastTriggered = 0;
    
    if (addTimer(timer)) {
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Failed to add timer\"}");
    }
}

void WebServer::handleTimerConfig() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Missing request body\"}");
        return;
    }
    
    String data = server.arg("plain");
    StaticJsonDocument<256> doc;  // 减小文档大小
    DeserializationError error = deserializeJson(doc, data);
    
    if (error) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }
    
    uint32_t id = doc["id"] | 0;
    Timer* timer = getTimer(id);
    if (!timer) {
        server.send(404, "application/json", "{\"error\":\"Timer not found\"}");
        return;
    }
    
    // 更新定时器属性
    if (doc.containsKey("relayId")) timer->relayId = doc["relayId"];
    if (doc.containsKey("hour")) timer->hour = doc["hour"];
    if (doc.containsKey("minute")) timer->minute = doc["minute"];
    if (doc.containsKey("enabled")) timer->enabled = doc["enabled"];
    if (doc.containsKey("state")) timer->state = doc["state"];
    if (doc.containsKey("repeat")) timer->repeat = static_cast<TimerRepeat>(doc["repeat"].as<int>());
    if (doc.containsKey("weekdays")) timer->weekdays = doc["weekdays"];
    
    if (updateTimer(*timer)) {
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Failed to update timer\"}");
    }
}

bool WebServer::addTimer(const Timer& timer) {
    if (timerCount >= MAX_TIMERS) {
        Serial.println("Timer list is full");
        return false;
    }
    
    // 检查ID是否已存在
    for (int i = 0; i < timerCount; i++) {
        if (timers[i].id == timer.id) {
            Serial.println("Timer ID already exists");
            return false;
        }
    }
    
    // 检查继电器ID是否有效
    if (timer.relayId >= 3) {
        Serial.println("Invalid relay ID");
        return false;
    }
    
    // 添加定时器
    timers[timerCount++] = timer;
    saveTimerConfig();  // 保存配置
    return true;
}

bool WebServer::deleteTimer(uint8_t id) {
    for (int i = 0; i < timerCount; i++) {
        if (timers[i].id == id) {
            // 移动后面的定时器
            for (int j = i; j < timerCount - 1; j++) {
                timers[j] = timers[j + 1];
            }
            timerCount--;
            saveTimerConfig();  // 保存配置
            return true;
        }
    }
    return false;
}

bool WebServer::updateTimer(const Timer& timer) {
    for (int i = 0; i < timerCount; i++) {
        if (timers[i].id == timer.id) {
            timers[i] = timer;
            saveTimerConfig();  // 保存配置
            return true;
        }
    }
    return false;
}

Timer* WebServer::getTimer(uint8_t id) {
    for (int i = 0; i < timerCount; i++) {
        if (timers[i].id == id) {
            return &timers[i];
        }
    }
    return nullptr;
}

bool WebServer::isWeekday(int weekday) {
    return weekday >= 1 && weekday <= 5;  // 1-5 表示周一到周五
}

bool WebServer::shouldTriggerTimer(const Timer& timer, const tm* timeinfo) {
    if (!timer.enabled) return false;
    
    // 检查时间是否匹配
    if (timeinfo->tm_hour != timer.hour || timeinfo->tm_min != timer.minute) {
        return false;
    }
    
    // 检查重复类型
    switch (timer.repeat) {
        case TimerRepeat::ONCE:
            // 只执行一次，检查是否已经执行过
            return timer.lastTriggered == 0;
            
        case TimerRepeat::DAILY:
            return true;
            
        case TimerRepeat::WEEKDAY:
            return isWeekday(timeinfo->tm_wday);
            
        case TimerRepeat::WEEKEND:
            return !isWeekday(timeinfo->tm_wday);
            
        case TimerRepeat::CUSTOM:
            // 检查是否在指定的星期几
            return (timer.weekdays & (1 << timeinfo->tm_wday)) != 0;
            
        default:
            return false;
    }
}

void WebServer::saveTimerConfig() {
    File configFile = SPIFFS.open("/timers.json", "w");
    if (!configFile) {
        Serial.println("Failed to open timers.json for writing");
        return;
    }
    
    StaticJsonDocument<1024> doc;  // 减小文档大小
    JsonArray timersArray = doc.createNestedArray("timers");
    
    for (int i = 0; i < timerCount; i++) {
        JsonObject timerObj = timersArray.createNestedObject();
        timerObj["id"] = timers[i].id;
        timerObj["relayId"] = timers[i].relayId;
        timerObj["hour"] = timers[i].hour;
        timerObj["minute"] = timers[i].minute;
        timerObj["enabled"] = timers[i].enabled;
        timerObj["state"] = timers[i].state;
        timerObj["repeat"] = static_cast<int>(timers[i].repeat);
        timerObj["weekdays"] = timers[i].weekdays;
        timerObj["lastTriggered"] = timers[i].lastTriggered;
    }
    
    if (serializeJson(doc, configFile) == 0) {
        Serial.println("Failed to write to timers.json");
    }
    configFile.close();
}

void WebServer::loadTimerConfig() {
    if (!SPIFFS.exists("/timers.json")) {
        Serial.println("No timer configuration found");
        return;
    }
    
    File configFile = SPIFFS.open("/timers.json", "r");
    if (!configFile) {
        Serial.println("Failed to open timers.json");
        return;
    }
    
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();
    
    if (error) {
        Serial.println("Failed to parse timers.json");
        return;
    }
    
    JsonArray timersArray = doc["timers"];
    timerCount = 0;
    
    for (JsonObject timerObj : timersArray) {
        if (timerCount >= MAX_TIMERS) break;
        
        Timer& timer = timers[timerCount++];
        timer.id = timerObj["id"];
        timer.relayId = timerObj["relayId"];
        timer.hour = timerObj["hour"];
        timer.minute = timerObj["minute"];
        timer.enabled = timerObj["enabled"];
        timer.state = timerObj["state"];
        timer.repeat = static_cast<TimerRepeat>(timerObj["repeat"].as<int>());
        timer.weekdays = timerObj["weekdays"];
        timer.lastTriggered = timerObj["lastTriggered"];
    }
    
    Serial.print("Loaded ");
    Serial.print(timerCount);
    Serial.println(" timers");
}

void WebServer::syncTime() {
    static unsigned long lastSync = 0;
    if (millis() - lastSync > 24 * 3600 * 1000) { // 每24小时同步一次
        configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
        lastSync = millis();
    }
}

void WebServer::handleDeleteTimer() {
    if (!server.hasArg("id")) {
        server.send(400, "application/json", "{\"error\":\"Missing timer ID\"}");
        return;
    }
    
    String idStr = server.arg("id");
    uint32_t id = idStr.toInt();
    
    Serial.print("Deleting timer with ID: ");
    Serial.println(id);
    
    if (deleteTimer(id)) {
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(404, "application/json", "{\"error\":\"Timer not found\"}");
    }
}

void WebServer::handleGetProtection() {
    StaticJsonDocument<256> doc;
    doc["channel1"] = currentLimits[0];
    doc["channel2"] = currentLimits[1];
    doc["channel3"] = currentLimits[2];
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void WebServer::handleSetProtection() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Missing request body\"}");
        return;
    }
    
    String data = server.arg("plain");
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, data);
    
    if (error) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }
    
    currentLimits[0] = doc["channel1"] | 0;
    currentLimits[1] = doc["channel2"] | 0;
    currentLimits[2] = doc["channel3"] | 0;
    
    // 保存到 SPIFFS
    saveProtectionConfig();
    
    server.send(200, "application/json", "{\"success\":true}");
}

void WebServer::handleProtectionStatus() {
    StaticJsonDocument<512> doc;
    
    for (int i = 0; i < 3; i++) {
        JsonObject channel = doc.createNestedObject("channel" + String(i + 1));
        channel["current"] = powerMonitor.getCurrent_mA(i);
        channel["triggered"] = protectionTriggered[i];
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void WebServer::saveProtectionConfig() {
    File configFile = SPIFFS.open("/protection.json", "w");
    if (!configFile) {
        Serial.println("Failed to open protection.json for writing");
        return;
    }
    
    StaticJsonDocument<256> doc;
    doc["channel1"] = currentLimits[0];
    doc["channel2"] = currentLimits[1];
    doc["channel3"] = currentLimits[2];
    
    if (serializeJson(doc, configFile) == 0) {
        Serial.println("Failed to write to protection.json");
    }
    configFile.close();
}

void WebServer::loadProtectionConfig() {
    if (!SPIFFS.exists("/protection.json")) {
        Serial.println("No protection configuration found");
        return;
    }
    
    File configFile = SPIFFS.open("/protection.json", "r");
    if (!configFile) {
        Serial.println("Failed to open protection.json");
        return;
    }
    
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();
    
    if (error) {
        Serial.println("Failed to parse protection.json");
        return;
    }
    
    currentLimits[0] = doc["channel1"] | 0;
    currentLimits[1] = doc["channel2"] | 0;
    currentLimits[2] = doc["channel3"] | 0;
}

void WebServer::checkCurrentProtection() {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck < 1000) return;  // 每秒检查一次
    lastCheck = millis();
    
    for (int i = 0; i < 3; i++) {
        if (currentLimits[i] > 0) {  // 只检查设置了限制的通道
            float current = powerMonitor.getCurrent_mA(i);
            if (current > currentLimits[i]) {
                if (!protectionTriggered[i]) {
                    protectionTriggered[i] = true;
                    // 关闭继电器
                    updateRelay(i, false);
                    Serial.print("Current protection triggered for channel ");
                    Serial.print(i + 1);
                    Serial.print(": ");
                    Serial.print(current);
                    Serial.println(" mA");
                }
            } 
        }
    }
}

void WebServer::handleGetConfigData() {
    StaticJsonDocument<512> doc;
    const Config& config = wifi.getConfig();
    
    doc["ssid"] = config.SSID;
    doc["passwd"] = config.Passwd;
    doc["server"] = config.Server;
    doc["topic"] = config.Topic;
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}