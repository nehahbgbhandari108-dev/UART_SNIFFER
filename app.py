from flask import Flask, request, jsonify, render_template, send_file
from pathlib import Path
import csv
from datetime import datetime
import os

app = Flask(__name__)
app.config['JSON_SORT_KEYS'] = False

BASE_DIR = Path(__file__).resolve().parent
CSV_FILE = BASE_DIR / 'logs.csv'


def ensure_csv_exists() -> None:
    if not CSV_FILE.exists():
        with CSV_FILE.open('w', newline='', encoding='utf-8') as file:
            writer = csv.writer(file)
            writer.writerow(["Time", "ChipID", "Message"])


def read_logs() -> list[dict[str, str]]:
    ensure_csv_exists()
    logs: list[dict[str, str]] = []

    with CSV_FILE.open('r', newline='', encoding='utf-8') as file:
        reader = csv.DictReader(file)
        for row in reader:
            if row:
                logs.append({
                    'time': row.get('Time', ''),
                    'chip': row.get('ChipID', ''),
                    'message': row.get('Message', ''),
                })

    return logs


def append_log(chip: str, message: str) -> None:
    ensure_csv_exists()
    now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

    with CSV_FILE.open('a', newline='', encoding='utf-8') as file:
        writer = csv.writer(file)
        writer.writerow([now, chip, message])

    print(f"[{now}] {chip}: {message}")


@app.route('/')
def home() -> str:
    logs = read_logs()
    logs.reverse()
    return render_template('index.html', logs=logs)


@app.route('/log', methods=['POST'])
def receive_log():
    try:
        data = request.get_json()

        chip = data.get('chip', 'UNKNOWN')
        message = data.get('log', '')

        now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

        with open(CSV_FILE, 'a', newline='') as file:
            writer = csv.writer(file)
            writer.writerow([now, chip, message])

        print(f"[{now}] {chip}: {message}")

        return jsonify({
            "status": "success"
        }), 200

    except Exception as e:
        return jsonify({
            "status": "error",
            "message": str(e)
        }), 500


@app.route('/download')
def download_csv():
    return send_file(CSV_FILE, as_attachment=True)


@app.route('/api/logs', methods=['GET'])
def get_logs_json():
    """API endpoint to get logs as JSON"""
    logs = read_logs()
    logs.reverse()
    return jsonify(logs), 200


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=False)