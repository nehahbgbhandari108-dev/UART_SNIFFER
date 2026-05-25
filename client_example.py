#!/usr/bin/env python3
"""
Example script to send UART log data to the UART Sniffer server.
Useful for testing and debugging.
"""

import requests
import json
import time
from datetime import datetime

# Server configuration
SERVER_URL = "http://localhost:5000"
ENDPOINT = f"{SERVER_URL}/log"

def send_log(chip_id: str, message: str) -> bool:
    """
    Send a log message to the UART Sniffer server.
    
    Args:
        chip_id: Identifier for the chip/device sending the log
        message: The log message content
        
    Returns:
        True if successful, False otherwise
    """
    try:
        data = {
            "chip": chip_id,
            "log": message
        }
        
        response = requests.post(
            ENDPOINT,
            json=data,
            headers={"Content-Type": "application/json"}
        )
        
        if response.status_code == 200:
            result = response.json()
            if result.get("status") == "success":
                print(f"✓ Sent: [{chip_id}] {message}")
                return True
            else:
                print(f"✗ Error: {result.get('message', 'Unknown error')}")
                return False
        else:
            print(f"✗ Server error: {response.status_code}")
            return False
            
    except requests.exceptions.ConnectionError:
        print(f"✗ Cannot connect to server at {SERVER_URL}")
        print("  Make sure the server is running: python app.py")
        return False
    except Exception as e:
        print(f"✗ Error: {str(e)}")
        return False


def get_logs() -> list:
    """
    Retrieve all logs from the server.
    
    Returns:
        List of log entries, or empty list if error
    """
    try:
        response = requests.get(f"{SERVER_URL}/api/logs")
        if response.status_code == 200:
            return response.json()
        else:
            print(f"Error retrieving logs: {response.status_code}")
            return []
    except requests.exceptions.ConnectionError:
        print("Cannot connect to server")
        return []
    except Exception as e:
        print(f"Error: {str(e)}")
        return []


def test_server() -> bool:
    """Test connection to the server."""
    try:
        response = requests.get(SERVER_URL)
        if response.status_code == 200:
            print(f"✓ Server is running at {SERVER_URL}")
            return True
        else:
            print(f"✗ Server returned status {response.status_code}")
            return False
    except requests.exceptions.ConnectionError:
        print(f"✗ Cannot connect to server at {SERVER_URL}")
        return False


def demo_mode():
    """Run demo mode with test data."""
    print("\n" + "="*60)
    print("UART Sniffer - Demo Mode")
    print("="*60 + "\n")
    
    if not test_server():
        return
    
    print("\nSending test messages...\n")
    
    test_messages = [
        ("ESP32-1", "System initialized"),
        ("ESP32-1", "WiFi connecting..."),
        ("ESP32-2", "Sensor reading: 23.5°C"),
        ("ESP32-1", "WiFi connected"),
        ("ESP32-2", "Sensor reading: 23.6°C"),
        ("ESP32-3", "Status: OK"),
        ("ESP32-1", "Server ready"),
    ]
    
    for chip, message in test_messages:
        send_log(chip, message)
        time.sleep(0.5)
    
    print("\n" + "-"*60)
    print("Recent logs:")
    print("-"*60 + "\n")
    
    logs = get_logs()
    if logs:
        for log in logs[-5:]:  # Show last 5 logs
            print(f"[{log['time']}] {log['chip']}: {log['message']}")
    else:
        print("No logs retrieved")
    
    print(f"\nOpen your browser to: {SERVER_URL}")
    print("Press Ctrl+C to exit demo mode\n")


if __name__ == "__main__":
    import sys
    
    if len(sys.argv) < 2:
        # Run demo mode if no arguments
        demo_mode()
    elif sys.argv[1] == "test":
        # Test connection
        test_server()
    elif sys.argv[1] == "logs":
        # Show recent logs
        logs = get_logs()
        if logs:
            print("\nRecent logs:")
            for log in logs[-10:]:
                print(f"[{log['time']}] {log['chip']}: {log['message']}")
        else:
            print("No logs available")
    else:
        # Send custom message
        chip = sys.argv[1]
        message = " ".join(sys.argv[2:]) if len(sys.argv) > 2 else "Test message"
        send_log(chip, message)
