from flask import Flask, request, jsonify, render_template, send_file
from pathlib import Path
import csv
from datetime import datetime
import os
import sqlite3
DB_FILE = "uart_logs.db"

app = Flask(__name__)
app.config['JSON_SORT_KEYS'] = False

BASE_DIR = Path(__file__).resolve().parent
CSV_FILE = BASE_DIR / 'logs.csv'

DEFAULT_PORT = int(os.environ.get('PORT', 5000))
def init_db():

    conn = sqlite3.connect(DB_FILE)

    conn.execute("""
    CREATE TABLE IF NOT EXISTS logs(
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        time TEXT,
        chip TEXT,
        source TEXT,
        message TEXT
    )
    """)

    conn.commit()
    conn.close()

init_db()
def read_logs():

    conn = sqlite3.connect(DB_FILE)

    cursor = conn.cursor()

    cursor.execute("""
        SELECT time, chip, source, message
        FROM logs
        ORDER BY id DESC
    """)

    rows = cursor.fetchall()

    conn.close()

    return [
        {
            "time": row[0],
            "chip": row[1],
            "source": row[2],
            "message": row[3]
        }
        for row in rows
    ]

def append_log(
    chip,
    source,
    message,
    timestamp=None
):

    if timestamp is None:

        timestamp = datetime.now().strftime(
            "%Y-%m-%d %H:%M:%S"
        )

    conn = sqlite3.connect(DB_FILE)

    conn.execute(
        """
        INSERT INTO logs
        (time, chip, source, message)
        VALUES (?, ?, ?, ?)
        """,
        (
            timestamp,
            chip,
            source,
            message
        )
    )

    conn.commit()
    conn.close()

@app.after_request
def add_header(response):
    response.headers['Cache-Control'] = 'no-cache, no-store, must-revalidate, public, max-age=0'
    response.headers['Pragma'] = 'no-cache'
    response.headers['Expires'] = '0'
    return response


@app.route('/')
def home() -> str:
    logs = read_logs()
    logs.reverse()
    return render_template('index.html', logs=logs)


@app.route('/log', methods=['POST'])
def receive_log():
    try:
        data = request.get_json(force=True, silent=True)
        if not data:
            raise ValueError('Invalid JSON payload')

        chip = data.get('chip', 'UNKNOWN')

        source = data.get(
            'source',
            'UNKNOWN'
        )

        message = data.get(
            'log',
            ''
        )

        timestamp = data.get(
            'time'
        )

        append_log(
            chip,
            source,
            message,
            timestamp
        )

        return jsonify({
            'status': 'success'
        }), 200

    except Exception as e:
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@app.route('/download')
def download_csv():

    conn = sqlite3.connect(DB_FILE)

    cursor = conn.cursor()

    cursor.execute("""
        SELECT
            time,
            chip,
            source,
            message
        FROM logs
        ORDER BY id DESC
    """)

    rows = cursor.fetchall()

    conn.close()

    csv_file = "logs.csv"

    with open(
        csv_file,
        'w',
        newline='',
        encoding='utf-8'
    ) as file:

        writer = csv.writer(file)

        writer.writerow([
            "Time",
            "ChipID",
            "Source",
            "UART Data"
        ])

        writer.writerows(rows)

    return send_file(
        csv_file,
        as_attachment=True
    )

@app.route('/api/logs', methods=['GET'])
def get_logs_json():
    """API endpoint to get logs as JSON"""
    logs = read_logs()
    logs.reverse()
    return jsonify(logs), 200


if __name__ == '__main__':

    init_db()

    app.run(
        host='0.0.0.0',
        port=5000,
        debug=False
    )