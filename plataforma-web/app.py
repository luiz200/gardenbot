import os
from flask import Flask, request, jsonify, render_template
from flask_mqtt import Mqtt
import ujson
from flask_bootstrap import Bootstrap
from dotenv import load_dotenv

load_dotenv()

app = Flask(__name__)

# Configurar o Flask para MQTT
app.config['MQTT_BROKER_URL'] = os.environ.get('MQTT_BROKER_URL')
app.config['MQTT_BROKER_PORT'] = int(os.environ.get('MQTT_BROKER_PORT'))
app.config['MQTT_USERNAME'] = os.environ.get('MQTT_USERNAME')
app.config['MQTT_PASSWORD'] = os.environ.get('MQTT_PASSWORD')
app.config['MQTT_KEEPALIVE'] = int(os.environ.get('MQTT_KEEPALIVE'))
app.config['MQTT_TLS_ENABLED'] = os.environ.get('MQTT_TLS_ENABLED').lower() == 'true'

topic_luminosity = "/luminosidade"
topic_temperature = "/temperatura"

mqtt_client = Mqtt(app)

latest_messages = {
    topic_temperature: None,
    topic_luminosity: None
}


@mqtt_client.on_connect()
def handle_connect(client, userdata, flags, rc):
    """
    The handle_connect function is called when the client connects to the broker.
    The client parameter is a reference to the client instance that has connected.
    The userdata parameter contains any user data that was passed as part of 
    the connection request, or None if no user data was specified. The flags 
    parameter contains response flags sent by the broker (not used in this example). 
    The rc parameter specifies whether or not the connection attempt succeeded.
    
    :param client: Pass the client instance to the callback function
    :param userdata: Pass user data to the callback function
    :param flags: Indicate the status of the connection
    :param rc: Check if the connection was successful
    :return: A code
    :doc-author: Trelent
    """
    if rc == 0:
        print('Connected successfully')
        mqtt_client.subscribe(topic_luminosity)
        mqtt_client.subscribe(topic_temperature)
    else:
        print('Bad connection. Code:', rc)


@mqtt_client.on_message()
def handle_mqtt_message(client, userdata, message):
    """
    The handle_mqtt_message function is called when a message is received on the MQTT topic.
    The function prints out the message and then publishes an acknowledgement to the ESP32.
    
    :param client: Pass in the client instance for the callback function to call
    :param userdata: Pass user data to callback functions
    :param message: Pass the message to the function
    :return: None
    :doc-author: Trelent
    """
    global latest_messages

    data = dict(
        topic=message.topic,
        payload=message.payload.decode()
    )
    if message.topic == topic_luminosity:
        data['topic_luminosity'] = message.payload.decode()
        latest_messages[topic_luminosity] = data['topic_luminosity']
    elif message.topic == topic_temperature:
        data['topic_temperature'] = message.payload.decode()
        latest_messages[topic_temperature] = data['topic_temperature']

    output = 'Received message on topic: {topic}'.format(**data)
    if 'topic_luminosity' in data:
        output += ', Luminosity: {topic_luminosity}'.format(**data)
    if 'topic_temperature' in data:
        output += ', Temperature: {topic_temperature}'.format(**data)
    output += ' with payload: {payload}'.format(**data)

    print(output)


@app.route('/')
def index():
    """
    The index function is the default function that Flask will call when a user visits the root URL of our website.
    It returns a rendered template, which we have called index.html and stored in the templates folder.
    
    :return: The index
    :doc-author: Trelent
    """
    return render_template('index.html', latest_temperature=latest_messages[topic_temperature], latest_luminosity=latest_messages[topic_luminosity])


@app.route('/get_latest_message', methods=['GET'])
def get_latest_message():
    """
    The get_latest_message function returns the latest message received from the MQTT broker.
    
    :return: A json object containing the latest messages of both topics
    :doc-author: Trelent
    """
    return jsonify({
        'message': {
            'temperature': latest_messages[topic_temperature],
            'luminosity': latest_messages[topic_luminosity],
        }
    })

if __name__ == '__main__':
    app.run(host='127.0.0.1', port=5000)