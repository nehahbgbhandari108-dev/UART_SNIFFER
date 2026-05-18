
from flask import Flask
from flask import request
from flask import render_template_string

from flask_socketio import SocketIO

app = Flask(__name__)

socketio = SocketIO(app,
                    cors_allowed_origins="*")

# ==========================================
# HOME PAGE
# ==========================================

@app.route('/')

def home():

    html = """

<!DOCTYPE html>
<html>

<head>

<title>UART LOGGER</title>

<script src="https://cdn.socket.io/4.5.4/socket.io.min.js"></script>

<style>

body{
background:black;
color:lime;
font-family:monospace;
padding:20px;
}

#log{
height:600px;
overflow:auto;
border:1px solid lime;
padding:10px;
white-space:pre-wrap;
}

</style>

</head>

<body>

<h2>ESP32 UART LOGGER</h2>

<div id="log"></div>

<script>

var socket = io();

socket.on('new_log', function(data){

    var log =
    document.getElementById('log');

    log.innerHTML += data + '\\n';

    log.scrollTop =
    log.scrollHeight;
});

</script>

</body>
</html>

"""

    return render_template_string(html)

# ==========================================
# RECEIVE LOGS
# ==========================================

@app.route('/log',
           methods=['POST'])

def log():

    data = request.json

    line = f'''
[{data["timestamp"]}]
GPIO      : {data["gpio"]}
DIRECTION : {data["direction"]}
CMD       : {data["cmd"]}
CHECKSUM  : {data["checksum"]}
DATA      : {data["data"]}
------------------------------------------------
'''

    print(line)

    socketio.emit(
        'new_log',
        line
    )

    return "OK"

# ==========================================
# START
# ==========================================

if __name__ == '__main__':

    socketio.run(app,
                 host='0.0.0.0',
                 port=5000)