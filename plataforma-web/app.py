import os
from flask import Flask, request, jsonify, render_template
from flask_mqtt import Mqtt
import ujson
from flask_bootstrap import Bootstrap
from dotenv import load_dotenv

load_dotenv()

app = Flask(__name__, static_url_path='/static')

# Configurar o Flask para MQTT
app.config['MQTT_BROKER_URL'] = os.environ.get('MQTT_BROKER_URL')
app.config['MQTT_BROKER_PORT'] = int(os.environ.get('MQTT_BROKER_PORT'))
app.config['MQTT_USERNAME'] = os.environ.get('MQTT_USERNAME')
app.config['MQTT_PASSWORD'] = os.environ.get('MQTT_PASSWORD')
app.config['MQTT_KEEPALIVE'] = int(os.environ.get('MQTT_KEEPALIVE'))
app.config['MQTT_TLS_ENABLED'] = os.environ.get('MQTT_TLS_ENABLED').lower() == 'true'

topic_umidade_ar = "/umidade_ar"
topic_umidade_solo1 = "/umidade_solo1"
topic_umidade_solo2 = "/umidade_solo2"
topic_umidade_solo3 = "/umidade_solo3"
topic_umidade_solo4 = "/umidade_solo4"
topic_temperature = "/temperatura"

mqtt_client = Mqtt(app)

latest_messages = {
    topic_temperature: None,
    topic_umidade_ar: None,
    topic_umidade_solo1: None,
    topic_umidade_solo2: None,
    topic_umidade_solo3: None,
    topic_umidade_solo4: None
}

previous_messages = {
    topic_temperature: None,
    topic_umidade_ar: None,
    topic_umidade_solo1: None,
    topic_umidade_solo2: None,
    topic_umidade_solo3: None,
    topic_umidade_solo4: None
}

@mqtt_client.on_connect()
def handle_connect(client, userdata, flags, rc):
    
    if rc == 0:
        print('Connected successfully')
        mqtt_client.subscribe(topic_umidade_ar)
        mqtt_client.subscribe(topic_umidade_solo1)
        mqtt_client.subscribe(topic_umidade_solo2)
        mqtt_client.subscribe(topic_umidade_solo3)
        mqtt_client.subscribe(topic_umidade_solo4)
        mqtt_client.subscribe(topic_temperature)
    else:
        print('Bad connection. Code:', rc)


@mqtt_client.on_message()
def handle_mqtt_message(client, userdata, message):

    global latest_messages, previous_messages

    data = dict(
        topic=message.topic,
        payload=message.payload.decode()
    )

    output = 'Received message on topic: {topic}'.format(**data)
    if 'topic_umidade_ar' in data:
        output += ', Umidade do ar: {topic_umidade_ar}'.format(**data)
    if 'topic_umidade_solo1' in data:
        output += ', Umidade solo1: {topic_umidade_solo1}'.format(**data)
    if 'topic_umidade_solo2' in data:
        output += ', Umidade solo2: {topic_umidade_solo2}'.format(**data)
    if 'topic_umidade_solo3' in data:
        output += ', Umidade solo3: {topic_umidade_solo3}'.format(**data)
    if 'topic_umidade_solo4' in data:
        output += ', Umidade solo4: {topic_umidade_solo4}'.format(**data)
    if 'topic_temperature' in data:
        output += ', Temperature: {topic_temperature}'.format(**data)

    output += ' with payload: {payload}'.format(**data)

    print(output)

def compare_values(current, previous):
    if previous is None:
        return "No data"
    
    if current > previous:
        return "subiu"
    elif current < previous:
        return "desceu"
    else:
        return "permaneceu o mesmo"


@app.route('/')
def index():

    return render_template('index.html', 
                        latest_temperature=latest_messages[topic_temperature], latest_umidade_ar=latest_messages[topic_umidade_ar],
                        latest_umidade_solo1=latest_messages[topic_umidade_solo1], latest_umidade_solo2=latest_messages[topic_umidade_solo2], 
                        latest_umidade_solo3=latest_messages[topic_umidade_solo3], latest_umidade_solo4=latest_messages[topic_umidade_solo4],
                        temperature_comparison=compare_values(latest_messages[topic_temperature], previous_messages[topic_temperature]),
                        umidade_ar_comparison=compare_values(latest_messages[topic_umidade_ar], previous_messages[topic_umidade_ar]),
                        umidade_solo1_comparison=compare_values(latest_messages[topic_umidade_solo1], previous_messages[topic_umidade_solo1]),
                        umidade_solo2_comparison=compare_values(latest_messages[topic_umidade_solo2], previous_messages[topic_umidade_solo2]),
                        umidade_solo3_comparison=compare_values(latest_messages[topic_umidade_solo3], previous_messages[topic_umidade_solo3]),
                        umidade_solo4_comparison=compare_values(latest_messages[topic_umidade_solo4], previous_messages[topic_umidade_solo4])
                        )


@app.route('/get_latest_message', methods=['GET'])
def get_latest_message():

    return jsonify({
        'message': {
            'temperature': latest_messages[topic_temperature],
            'temperature_comparison': compare_values(latest_messages[topic_temperature], previous_messages[topic_temperature]),
            'umidade_ar': latest_messages[topic_umidade_ar],
            'umidade_ar_comparison': compare_values(latest_messages[topic_umidade_ar], previous_messages[topic_umidade_ar]),
            'umidade_solo1': latest_messages[topic_umidade_solo1],
            'umidade_solo1_comparison': compare_values(latest_messages[topic_umidade_solo1], previous_messages[topic_umidade_solo1]),
            'umidade_solo2': latest_messages[topic_umidade_solo2],
            'umidade_solo2_comparison': compare_values(latest_messages[topic_umidade_solo2], previous_messages[topic_umidade_solo2]),
            'umidade_solo3': latest_messages[topic_umidade_solo3],
            'umidade_solo3_comparison': compare_values(latest_messages[topic_umidade_solo3], previous_messages[topic_umidade_solo3]),
            'umidade_solo4': latest_messages[topic_umidade_solo4],
            'umidade_solo4_comparison': compare_values(latest_messages[topic_umidade_solo4], previous_messages[topic_umidade_solo4])
        }
    })

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080, debug=True)