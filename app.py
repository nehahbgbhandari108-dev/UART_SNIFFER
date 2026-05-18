from flask import Flask, request, jsonify, render_template
import csv
import os
from datetime import datetime

app = Flask(__name__)

CSV_FILE = 'logs.csv'

# Create CSV file if not exists
if not os.path.exists(CSV_FILE):
    with open(CSV_FILE, 'w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(["Time", "ChipID", "Message"])


@app.route('/')
def home():
    logs = []

    with open(CSV_FILE, 'r') as file:
        reader = csv.reader(file)
        next(reader)

        for row in reader:
            logs.append(row)

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
    from flask import send_file
    return send_file(CSV_FILE, as_attachment=True)


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)