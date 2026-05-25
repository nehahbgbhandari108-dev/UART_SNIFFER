@echo off
REM UART Sniffer - Setup and Run Script for Windows

echo.
echo ============================================
echo    UART Sniffer - Setup and Installation
echo ============================================
echo.

REM Check if Python is installed
python --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Python is not installed or not in PATH
    echo Please install Python 3.8+ from https://www.python.org/
    echo Make sure to check "Add Python to PATH" during installation
    pause
    exit /b 1
)

echo [1/3] Checking Python installation...
python --version
echo.

echo [2/3] Installing dependencies...
pip install -r requirements.txt
if errorlevel 1 (
    echo ERROR: Failed to install dependencies
    pause
    exit /b 1
)
echo.

echo [3/3] Setup complete!
echo.
echo ============================================
echo      Starting UART Sniffer Server...
echo ============================================
echo.
echo Access the web interface at:
echo   http://localhost:5000
echo.
echo From other machines on your network:
echo   http://^<your-computer-ip^>:5000
echo.
echo Press Ctrl+C to stop the server
echo.

python app.py
