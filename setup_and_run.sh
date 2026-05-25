#!/bin/bash

# UART Sniffer - Setup and Run Script for Linux/Mac

echo ""
echo "============================================"
echo "    UART Sniffer - Setup and Installation"
echo "============================================"
echo ""

# Check if Python is installed
if ! command -v python3 &> /dev/null; then
    echo "ERROR: Python 3 is not installed"
    echo "Install with: brew install python3 (Mac) or apt-get install python3 (Linux)"
    exit 1
fi

echo "[1/3] Checking Python installation..."
python3 --version
echo ""

echo "[2/3] Installing dependencies..."
pip3 install -r requirements.txt
if [ $? -ne 0 ]; then
    echo "ERROR: Failed to install dependencies"
    exit 1
fi
echo ""

echo "[3/3] Setup complete!"
echo ""
echo "============================================"
echo "      Starting UART Sniffer Server..."
echo "============================================"
echo ""
echo "Access the web interface at:"
echo "  http://localhost:5000"
echo ""
echo "From other machines on your network:"
echo "  http://<your-computer-ip>:5000"
echo ""
echo "Press Ctrl+C to stop the server"
echo ""

python3 app.py
