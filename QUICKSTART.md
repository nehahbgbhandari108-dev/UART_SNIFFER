# UART Sniffer - Quick Start Guide

## 🚀 Get Started in 3 Minutes

### Step 1: Install Dependencies
```bash
pip install -r requirements.txt
```

### Step 2: Start the Server
```bash
python app.py
```

You should see:
```
 * Running on http://0.0.0.0:5000
```

### Step 3: Open in Browser
Open your browser and go to: **http://localhost:5000**

You should see a beautiful dashboard ready to receive logs!

---

## 📱 Access from Other Devices

Once the server is running, any device on your network can access it:

1. Find your computer's IP address:
   - **Windows**: Open Command Prompt and type `ipconfig` → look for IPv4 Address
   - **Mac/Linux**: Open Terminal and type `ifconfig` → look for inet

2. On another device, go to: **http://YOUR_IP:5000**
   - Example: `http://192.168.1.100:5000`

---

## 📨 Send Data to Server

### Option 1: Using the Example Script
```bash
# Demo mode with test data
python client_example.py

# Send custom message
python client_example.py ESP32-1 "Hello World"

# View recent logs
python client_example.py logs

# Test connection
python client_example.py test
```

### Option 2: Using curl
```bash
curl -X POST http://localhost:5000/log \
  -H "Content-Type: application/json" \
  -d '{"chip": "ESP32-1", "log": "Temperature: 25.5C"}'
```

### Option 3: From Python Code
```python
import requests

response = requests.post('http://localhost:5000/log', json={
    "chip": "ESP32-Board",
    "log": "Sensor reading: 42.5"
})
print(response.json())
```

### Option 4: From ESP32/Arduino
```cpp
#include <WiFi.h>
#include <HTTPClient.h>

void sendLog(String chipId, String message) {
    HTTPClient http;
    http.begin("http://192.168.1.100:5000/log");
    http.addHeader("Content-Type", "application/json");
    
    String payload = "{\"chip\":\"" + chipId + "\",\"log\":\"" + message + "\"}";
    int httpResponseCode = http.POST(payload);
    
    Serial.println(httpResponseCode);
    http.end();
}

void setup() {
    // ... WiFi setup ...
    sendLog("ESP32-1", "System started");
}
```

---

## 📊 Features

✅ **Real-time Dashboard** - Beautiful web interface  
✅ **Auto-refresh** - Updates every 5 seconds  
✅ **Download CSV** - Export all logs  
✅ **REST API** - Get logs as JSON  
✅ **Network Access** - Works across your network  
✅ **No Database** - Simple CSV storage  

---

## 🔗 API Endpoints

### GET /
Main dashboard (HTML)

### POST /log
Send a log entry
```json
{
  "chip": "ESP32-1",
  "log": "Your message"
}
```

### GET /api/logs
Get all logs as JSON
```bash
curl http://localhost:5000/api/logs
```

### GET /download
Download logs as CSV file

---

## ⚙️ Configuration

### Change Port (default 5000)
Edit `app.py`:
```python
app.run(host='0.0.0.0', port=8000, debug=False)
```

### Change Auto-refresh Interval
Edit `templates/index.html`, find this line and change `5000`:
```javascript
setInterval(() => {
    location.reload();
}, 5000);  // Change this number (milliseconds)
```

### Clear Logs
Simply delete or rename `logs.csv` - a new one will be created automatically

---

## 🐛 Troubleshooting

**"Port already in use"**
- Change the port number in app.py
- Or kill the process using the port

**"Cannot connect from another machine"**
- Make sure both devices are on the same network
- Check your firewall allows port 5000
- Use your actual IP, not `localhost` or `127.0.0.1`

**"Server not responding"**
- Make sure `python app.py` is still running
- Check the terminal for any error messages
- Try accessing http://localhost:5000

---

## 🚀 Production Deployment

For better performance in production:
```bash
gunicorn -w 4 -b 0.0.0.0:5000 app:app
```

Or use a reverse proxy (nginx, Apache) to handle traffic.

---

## 📝 Log Format

All logs are stored in `logs.csv` with three columns:
- **Time**: Timestamp when log was received
- **ChipID**: Identifier of the device sending the log
- **Message**: The log message content

---

## Need Help?

Check `README.md` for detailed documentation or review `client_example.py` for more usage examples.

Happy sniffing! 🎉
