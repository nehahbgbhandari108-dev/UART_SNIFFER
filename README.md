# UART Sniffer - Web Interface

A Flask-based web application for viewing and logging UART (serial) messages in real-time.

## Features

- **Real-time Logging**: Capture UART messages from embedded devices
- **Web Dashboard**: Beautiful, responsive public interface
- **Auto-refresh**: Pages refresh every 5 seconds to show latest logs
- **CSV Download**: Export all logs as CSV file
- **REST API**: Endpoints for programmatic access
- **Network Access**: Accessible from any device on the network

## Requirements

- Python 3.8+
- Flask 3.0.3
- Gunicorn 22.0.0

## Installation

1. Install dependencies:
```bash
pip install -r requirements.txt
```

## Usage

### Running the Server

```bash
python app.py
```

The web interface will be available at:
- Local: `http://localhost:5000`
- Network: `http://<your-ip>:5000` (accessible from any device on the network)

### Sending UART Data

Send a POST request to the `/log` endpoint with JSON data:

```bash
curl -X POST http://localhost:5000/log \
  -H "Content-Type: application/json" \
  -d '{"chip": "ESP32-1", "log": "Hello from UART"}'
```

Or in Python:
```python
import requests

data = {
    "chip": "ESP32-1",
    "log": "Your message here"
}

response = requests.post('http://localhost:5000/log', json=data)
print(response.json())
```

### Available Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/` | GET | Main dashboard (HTML) |
| `/log` | POST | Submit new UART log entry |
| `/api/logs` | GET | Get all logs as JSON |
| `/download` | GET | Download logs as CSV file |

## API Response Examples

### POST /log
**Request:**
```json
{
  "chip": "ESP32-1",
  "log": "Temperature: 25.5°C"
}
```

**Response (Success):**
```json
{
  "status": "success"
}
```

**Response (Error):**
```json
{
  "status": "error",
  "message": "error details"
}
```

### GET /api/logs
**Response:**
```json
[
  {
    "time": "2026-05-20 14:30:45",
    "chip": "ESP32-1",
    "message": "System started"
  },
  {
    "time": "2026-05-20 14:30:46",
    "chip": "ESP32-1",
    "message": "Connected to WiFi"
  }
]
```

## Configuration

- **Port**: Default 5000 (can be changed in `app.py`)
- **Host**: 0.0.0.0 (accessible from network)
- **Auto-refresh**: 5 seconds (configured in HTML template)

## File Structure

```
uart_sniffer/
├── app.py                  # Flask application
├── logs.csv               # CSV file with all logs
├── requirements.txt       # Python dependencies
├── templates/
│   └── index.html        # Main dashboard HTML
└── static/               # Static files (if needed)
```

## Development vs Production

### Development (Current)
```bash
python app.py
```

### Production (Recommended)
```bash
gunicorn -w 4 -b 0.0.0.0:$PORT app:app
```

### External Access with ngrok
If you want temporary external access from outside your LAN, install `ngrok` and run:
```bash
ngrok http 5000
```
Then open the HTTPS forwarding URL shown by ngrok.

## Quick Deploy Commands

Create a GitHub repo and push your code (if not already in a repo):
```bash
git init
git add .
git commit -m "Initial commit"
git branch -M main
git remote add origin https://github.com/<your-username>/<repo-name>.git
git push -u origin main
```

Build and run with Docker locally:
```bash
docker build -t uart-sniffer .
docker run -p 5000:5000 -v $(pwd)/logs.csv:/app/logs.csv uart-sniffer
```

Start with Gunicorn (production):
```bash
PORT=5000 gunicorn -w 4 -b 0.0.0.0:$PORT app:app
```

## Troubleshooting

### Port already in use
Change the port in `app.py`:
```python
app.run(host='0.0.0.0', port=8000, debug=False)
```

### Cannot access from other machines
Ensure:
1. Firewall allows port 5000
2. Using correct IP address (not localhost)
3. Both devices on same network

### Logs not appearing
- Check browser auto-refresh is enabled (happens every 5 seconds)
- Verify POST requests are being sent correctly
- Check Flask console output for errors

## License

MIT License
