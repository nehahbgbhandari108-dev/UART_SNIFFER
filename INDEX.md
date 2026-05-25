# UART Sniffer - Complete Documentation Index

Welcome to UART Sniffer! A modern, public-facing web interface for real-time UART/serial log monitoring.

---

## 📚 Documentation Files

### Getting Started
- **[QUICKSTART.md](QUICKSTART.md)** ⭐ **START HERE** - Get up and running in 3 minutes
- **[README.md](README.md)** - Complete feature overview and API documentation

### Advanced Topics
- **[DEPLOYMENT.md](DEPLOYMENT.md)** - Deploy to cloud platforms (Render, Docker, AWS, etc.)

---

## 🚀 Quick Installation

### Windows Users
Double-click `setup_and_run.bat` - it will install dependencies and start the server!

### Mac/Linux Users
```bash
chmod +x setup_and_run.sh
./setup_and_run.sh
```

### Manual Installation
```bash
pip install -r requirements.txt
python app.py
```

Then open: **http://localhost:5000**

---

## 📁 Project Structure

```
uart-sniffer/
├── 📄 README.md                 # Full documentation
├── 📄 QUICKSTART.md            # Quick start guide
├── 📄 DEPLOYMENT.md            # Cloud deployment guide
├── 📄 INDEX.md                 # This file
│
├── 🐍 app.py                   # Main Flask application
├── 🐍 client_example.py        # Example client script
│
├── 📋 requirements.txt         # Python dependencies
├── 📋 render.yaml             # Render.com deployment config
│
├── 🪟 setup_and_run.bat       # Windows quick start
├── 🐧 setup_and_run.sh        # Mac/Linux quick start
│
├── 📊 logs.csv                # Log storage (auto-created)
│
├── 📁 templates/
│   └── index.html             # Web dashboard (responsive UI)
└── 📁 static/                 # Static files (images, CSS, etc.)
```

---

## 💡 Features

✅ **Real-time Web Dashboard** - Beautiful, responsive interface  
✅ **Auto-refresh** - Updates every 5 seconds  
✅ **Network Access** - Works across your network  
✅ **REST API** - JSON endpoints for programmatic access  
✅ **CSV Export** - Download all logs  
✅ **No Database** - Simple CSV storage, no setup needed  
✅ **Production Ready** - Gunicorn + Flask  
✅ **Cloud Deployable** - Render, Docker, AWS, etc.  

---

## 📊 API Quick Reference

| Endpoint | Method | Purpose |
|----------|--------|---------|
| `/` | GET | Web dashboard |
| `/log` | POST | Submit log entry |
| `/api/logs` | GET | Get logs as JSON |
| `/download` | GET | Download CSV |

### Send Data
```bash
curl -X POST http://localhost:5000/log \
  -H "Content-Type: application/json" \
  -d '{"chip": "ESP32", "log": "Hello!"}'
```

---

## 🔍 Common Tasks

### View Logs
1. Open browser: `http://localhost:5000`
2. Logs auto-refresh every 5 seconds

### Send Test Messages
```bash
python client_example.py          # Demo mode
python client_example.py ESP32-1 "Test message"
python client_example.py logs     # View recent
```

### Download Logs
1. Visit `http://localhost:5000`
2. Click "📥 Download CSV" button
3. Or use: `curl http://localhost:5000/download`

### Access from Another Computer
1. Find your IP: `ipconfig` (Windows) or `ifconfig` (Mac/Linux)
2. Visit: `http://YOUR_IP:5000` from another device

### Clear All Logs
Delete `logs.csv` - a new one will auto-create on next log entry

### Change Port
Edit `app.py`, change:
```python
app.run(host='0.0.0.0', port=8000, debug=False)
```

### Change Auto-refresh Interval
Edit `templates/index.html`, change the `5000` milliseconds value:
```javascript
setInterval(() => {
    location.reload();
}, 5000);  // Change this
```

---

## 📡 Integration Examples

### Python
```python
import requests
requests.post('http://localhost:5000/log', json={
    "chip": "ESP32",
    "log": "Temperature: 25.5C"
})
```

### Arduino/ESP32
```cpp
HTTPClient http;
http.begin("http://192.168.1.100:5000/log");
http.addHeader("Content-Type", "application/json");
http.POST("{\"chip\":\"ESP32\",\"log\":\"Hello\"}");
```

### JavaScript
```javascript
fetch('http://localhost:5000/log', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({chip: 'ESP32', log: 'Hello!'})
})
```

### curl
```bash
curl -X POST http://localhost:5000/log \
  -H "Content-Type: application/json" \
  -d '{"chip":"ESP32","log":"Hello"}'
```

---

## 🌐 Deployment Options

### Option 1: Local Network (Easiest)
- Run on your computer
- Access from anywhere on your network
- No cloud account needed
- Computer must stay on

### Option 2: Render.com (Recommended)
- Free hosting with HTTPS
- Auto-deploys from GitHub
- 15-minute idle timeout on free tier
- See [DEPLOYMENT.md](DEPLOYMENT.md)

### Option 3: Docker
- Container-based deployment
- Run anywhere Docker runs
- Perfect for servers

### Option 4: Cloud Platforms
- AWS, Google Cloud, Azure
- More complex setup
- Full control and scaling
- See [DEPLOYMENT.md](DEPLOYMENT.md)

---

## 🆘 Troubleshooting

### "Port 5000 already in use"
Edit `app.py` and change the port number:
```python
app.run(host='0.0.0.0', port=8000, debug=False)
```

### "Cannot connect from other devices"
- Use your actual IP address, not localhost
- Check firewall allows port 5000
- Both devices must be on same network
- Find IP: `ipconfig` (Windows) or `ifconfig` (Mac/Linux)

### "Logs not showing"
- Refresh browser (or wait 5 seconds for auto-refresh)
- Check if `/log` endpoint received the data
- Look for errors in terminal where server is running

### "Server won't start"
- Make sure dependencies are installed: `pip install -r requirements.txt`
- Check Python version: `python --version` (need 3.8+)
- Try a different port if 5000 is busy

### "Imports not found"
- Make sure virtual environment is activated
- Reinstall: `pip install -r requirements.txt --force-reinstall`

---

## 📝 Log Format

All logs stored in `logs.csv`:

```
Time,ChipID,Message
2026-05-20 14:30:45,ESP32-1,System initialized
2026-05-20 14:30:46,ESP32-1,WiFi connecting...
2026-05-20 14:30:47,ESP32-1,WiFi connected
```

---

## 🔐 Security Notes

- **For Local Network Only**: No authentication needed for private networks
- **For Public Internet**: 
  - Add authentication (see [DEPLOYMENT.md](DEPLOYMENT.md))
  - Use HTTPS
  - Whitelist IPs or use VPN
  - Rate limiting on /log endpoint

---

## 📊 Performance

- **Small deployments** (< 1000 logs/day): Current setup is fine
- **Medium** (1000-10000 logs/day): Upgrade to Render paid or Docker
- **Large** (> 10000 logs/day): Switch to PostgreSQL database

---

## 🛠️ Development

### Project Stack
- **Backend**: Python Flask
- **Frontend**: HTML/CSS/JavaScript (Responsive)
- **Storage**: CSV (auto-created)
- **Server**: Gunicorn
- **Deployment**: Render, Docker, Cloud

### Adding Features
1. Edit `app.py` for backend routes
2. Edit `templates/index.html` for frontend
3. Restart server to test

---

## 📞 Getting Help

1. Check [README.md](README.md) for detailed docs
2. See [QUICKSTART.md](QUICKSTART.md) for common tasks
3. Review [DEPLOYMENT.md](DEPLOYMENT.md) for deployment issues
4. Check the terminal output for error messages
5. Review `client_example.py` for integration examples

---

## 🎉 Next Steps

1. **Get Started**: Follow [QUICKSTART.md](QUICKSTART.md)
2. **Send First Log**: Use `client_example.py` or curl
3. **View Dashboard**: Open `http://localhost:5000`
4. **Integrate**: Connect your ESP32/Arduino
5. **Deploy**: Use [DEPLOYMENT.md](DEPLOYMENT.md) for cloud hosting

---

**Enjoy your UART Sniffer! 🚀**

Happy logging! 📊
