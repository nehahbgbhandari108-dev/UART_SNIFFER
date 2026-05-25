# UART Sniffer - Deployment Guide

This guide covers deploying your UART Sniffer application to various platforms.

---

## 🏠 Local Network Deployment

The simplest setup - run on your local machine and access from anywhere on your network.

### Requirements
- Python 3.8+
- Same network as devices sending logs

### Steps
```bash
pip install -r requirements.txt
python app.py
```

Access from other devices at: `http://YOUR_IP:5000`

**Pros:**
- No cloud costs
- Instant setup
- Data stays local

**Cons:**
- Computer must stay on
- Cannot access remotely outside network
- No HTTPS

---

## 🚀 Render.com Deployment (Free)

Deploy your app online with automatic HTTPS and easy management.

### Prerequisites
- GitHub account (free)
- Render account (free at https://render.com)
- Your code pushed to GitHub

### Steps

1. **Push to GitHub**
   ```bash
   git init
   git add .
   git commit -m "Initial commit"
   git push origin main
   ```

2. **Create Render Account**
   - Go to https://render.com
   - Sign up with GitHub

3. **Deploy New Service**
   - Click "New +" → "Web Service"
   - Connect your GitHub repository
   - Use these settings:
     - **Name:** uart-sniffer
     - **Environment:** Python 3.11
     - **Build Command:** `pip install -r requirements.txt`
     - **Start Command:** `gunicorn -w 2 -b 0.0.0.0:10000 app:app`

4. **Environment Variables** (optional)
   - No additional variables needed for basic setup

5. **Deploy**
   - Click "Create Web Service"
   - Wait for deployment (2-3 minutes)
   - Your app will be at: `https://uart-sniffer.onrender.com`

### Notes
- Free tier has 15-minute idle timeout
- Auto-deploys when you push to GitHub
- HTTPS included

---

## 🌐 External Access via ngrok
If you need a quick public URL without pushing to cloud or changing network settings, use `ngrok`.

### Install ngrok
- Download from https://ngrok.com/
- Sign in and configure your auth token

### Run ngrok
```bash
grok http 5000
```

Open the public HTTPS URL shown in the ngrok terminal. The app remains live as long as ngrok is running.

> This is useful for temporary external access, testing, or sharing your dashboard with remote collaborators.

---

## 🔁 One-click / render.yaml Deploy (Render)

This repository includes a `render.yaml` manifest which Render will automatically use to create services when you import the repository.

Steps:

1. Push this repository to GitHub.

2. On Render (https://render.com), sign in and click **New + → Import from GitHub**.

3. Select this repository. Render will detect `render.yaml` and show a preview of the services to create.

4. Click **Create** (or **Create All**) to provision the service. Render will build, deploy, and provide an HTTPS URL.

Notes:
- The provided `render.yaml` in this repo already configures a web service using Python and Gunicorn.
- You can customize `render.yaml` in the repo before pushing to change region, plan, or start commands.
- Render automatically uses the `PORT` environment variable when starting Gunicorn; the `Procfile` is also present for other platforms.

If you prefer not to use `render.yaml`, you can create a Web Service manually in Render and use the following settings:

- Environment: `Python 3.11`
- Build Command: `pip install -r requirements.txt`
- Start Command: `gunicorn -w 2 -b 0.0.0.0:$PORT app:app`


## 🐳 Docker Deployment

Run in a Docker container for consistent environments.

### Create Dockerfile

```dockerfile
FROM python:3.11-slim

WORKDIR /app

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY . .

EXPOSE 5000

CMD ["gunicorn", "-w", "4", "-b", "0.0.0.0:5000", "app:app"]
```

### Create .dockerignore

```
__pycache__
*.pyc
.git
.gitignore
.env
```

### Build and Run

```bash
# Build
docker build -t uart-sniffer .

# Run locally
docker run -p 5000:5000 uart-sniffer

# Run with volume persistence
docker run -p 5000:5000 -v $(pwd)/logs.csv:/app/logs.csv uart-sniffer
```

---

## ☁️ Other Cloud Platforms

### Heroku (Legacy - Paid)
```bash
heroku login
heroku create uart-sniffer
git push heroku main
```

### AWS
Use Elastic Beanstalk or EC2 with Gunicorn

### Google Cloud
Use App Engine or Cloud Run

### Azure
Use App Service or Container Instances

---

## 📝 Production Checklist

Before going live:

- [ ] Set `debug=False` in app.py (already done)
- [ ] Use Gunicorn instead of Flask dev server
- [ ] Set up HTTPS/SSL certificate
- [ ] Configure firewall rules
- [ ] Set up log rotation (logs.csv might grow large)
- [ ] Add authentication if exposing publicly
- [ ] Monitor server resources
- [ ] Set up backups for logs.csv
- [ ] Use environment variables for secrets

---

## 🔐 Security Considerations

### If Exposing Publicly

1. **Add Authentication**
   ```python
   from flask_httpauth import HTTPBasicAuth
   auth = HTTPBasicAuth()
   
   @auth.verify_password
   def verify_password(username, password):
       return username == "admin" and password == "secret"
   
   @app.route('/')
   @auth.login_required
   def home():
       # ... rest of code
   ```

2. **Rate Limiting**
   ```bash
   pip install Flask-Limiter
   ```

3. **HTTPS Only**
   - Use Let's Encrypt (free)
   - Configure your reverse proxy

4. **Input Validation**
   - Sanitize chip IDs and messages
   - Limit message length

5. **Access Control**
   - Whitelist IP addresses
   - Use VPN for remote access

---

## 📈 Scaling

### For High Log Volume

1. **Use PostgreSQL instead of CSV**
   ```python
   from flask_sqlalchemy import SQLAlchemy
   ```

2. **Add Redis for caching**
   ```bash
   pip install redis
   ```

3. **Use multiple Gunicorn workers**
   ```bash
   gunicorn -w 8 app:app
   ```

4. **Add database indexing**

---

## 🧹 Maintenance

### Clearing Old Logs

```python
# Add to app.py
@app.route('/admin/clear-logs', methods=['POST'])
def clear_logs():
    if request.headers.get('X-Admin-Token') != 'your_secret_token':
        return jsonify({"error": "Unauthorized"}), 401
    
    CSV_FILE.unlink()
    ensure_csv_exists()
    return jsonify({"status": "cleared"}), 200
```

### Log Rotation

```bash
# Linux cron job
0 0 * * 0 mv /path/to/logs.csv /path/to/logs.backup.csv && touch /path/to/logs.csv
```

---

## 🆘 Troubleshooting Deployments

**App crashes after deployment:**
- Check logs: `https://dashboard.render.com/logs`
- Verify all dependencies in requirements.txt
- Check for missing environment variables

**"502 Bad Gateway" error:**
- Server is overloaded or crashed
- Check Render logs
- Increase worker count

**Logs not persisting:**
- CSV file resets on deploy
- Use a database for persistent storage
- Mount a persistent volume

**Very slow performance:**
- Free tier has limited resources
- Upgrade to paid plan
- Optimize database queries

---

## 📞 Support

For platform-specific help:
- **Render**: https://render.com/docs
- **Docker**: https://docs.docker.com
- **Gunicorn**: https://gunicorn.org
- **Flask**: https://flask.palletsprojects.com
