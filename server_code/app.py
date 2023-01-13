import eventlet
import json
from flask import Flask, render_template
from flask_mqtt import Mqtt
from flask_socketio import *
#SocketIO, send, emit, fn
from flask_bootstrap import Bootstrap
import ssl

eventlet.monkey_patch()

app = Flask(__name__)
app.config['TEMPLATES_AUTO_RELOAD'] = True
app.config['MQTT_BROKER_URL'] = 'massimog.net'
app.config['MQTT_USERNAME'] = ''
app.config['MQTT_PASSWORD'] = ''
app.config['MQTT_KEEPALIVE'] = 5

# Parameters for SSL enabled
app.config['MQTT_BROKER_PORT'] = 8883
app.config['MQTT_TLS_ENABLED'] = True
app.config['MQTT_TLS_INSECURE'] = True
app.config['MQTT_TLS_CA_CERTS'] = 'server.crt'
''' Blijkbaar niet nodig 
#app.config['MQTT_TLS_CERTFILE'] = 'Mosquitto/client.crt'
#app.config['MQTT_TLS_KEYFILE'] = 'Mosquitto/client.key'
#app.config['CLIENT_ID'] = 'Server-code'
'''
app.config['MQTT_TLS_VERSION'] = ssl.PROTOCOL_TLSv1_2


mqtt = Mqtt(app)
socketio = SocketIO(app)
bootstrap = Bootstrap(app)


@app.route('/')
def index():
    return render_template('index.html')

@socketio.on('publish')
def handle_publish(json_str):
    print('Publishing '+json_str)
    data = json.loads(json_str)
    mqtt.publish(data['topic'], data['message'])
    return "ok"

@socketio.on('subscribe')
def handle_subscribe(json_str):
    data = json.loads(json_str)
    print("Subscribing to "+ data['topic'])
    mqtt.subscribe(data['topic'])
    return "ok"

@socketio.on('unsubscribe_all')
def handle_unsubscribe_all():
    mqtt.unsubscribe_all()

'''
Relay MQTT messages to browsers with WebSockets
'''
@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    print('MQTT message from ' + message.topic)
    socketio.emit(message.topic, message.payload.decode())    

@mqtt.on_log()
def handle_logging(client, userdata, level, buf):
    print(level, buf)

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000,
                 use_reloader=False, debug=True)
