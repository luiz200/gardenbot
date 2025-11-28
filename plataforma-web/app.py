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
topic_chuva = "/chuva"
topic_intensidade_chuva = "/intensidade_chuva"

mqtt_client = Mqtt(app)

latest_messages = {
    topic_temperature: None,
    topic_umidade_ar: None,
    topic_umidade_solo1: None,
    topic_umidade_solo2: None,
    topic_umidade_solo3: None,
    topic_umidade_solo4: None,
    topic_chuva: None,
    topic_intensidade_chuva: None,
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
        mqtt_client.subscribe(topic_chuva)
        mqtt_client.subscribe(topic_intensidade_chuva)
    else:
        print('Bad connection. Code:', rc)


@mqtt_client.on_message()
def handle_mqtt_message(client, userdata, message):
    global latest_messages

    data = message.payload.decode()
    topic = message.topic

    latest_messages[topic] = data

    print(f'Mensagem recebida no tópico: {topic} com payload: {data}')

def intensidade():
    last_intensidade_chuva = latest_messages[topic_intensidade_chuva]
    if last_intensidade_chuva < 1000:
        return 'Chuva intensa'
    elif last_intensidade_chuva <= 3000 or last_intensidade_chuva >= 1000:
        return 'Chuva Moderada ou Chuvisco'
    elif last_intensidade_chuva > 4000:
        return 'Sem previsão de chuva'

def chuva():
    last_chuva = latest_messages[topic_chuva]
    if last_chuva == 1:
        return 'Sim'
    elif last_chuva == 0:
        return 'Não'


@app.route('/')
def index():
    chuvas = chuva()
    intensidades = intensidade()
    return render_template('index.html',
                           latest_temperature=latest_messages[topic_temperature],
                           latest_umidade_ar=latest_messages[topic_umidade_ar],
                           latest_umidade_solo1=latest_messages[topic_umidade_solo1],
                           latest_umidade_solo2=latest_messages[topic_umidade_solo2],
                           latest_umidade_solo3=latest_messages[topic_umidade_solo3],
                           latest_umidade_solo4=latest_messages[topic_umidade_solo4],
                           chuvas=chuvas,
                           intensidades=intensidades,
                           )


@app.route('/get_latest_message', methods=['GET'])
def get_latest_message():
    return jsonify({
        'message': {
            'temperature': latest_messages[topic_temperature],
            'umidade_ar': latest_messages[topic_umidade_ar],
            'umidade_solo1': latest_messages[topic_umidade_solo1],
            'umidade_solo2': latest_messages[topic_umidade_solo2],
            'umidade_solo3': latest_messages[topic_umidade_solo3],
            'umidade_solo4': latest_messages[topic_umidade_solo4],
            'chuva': latest_messages[topic_chuva],
            'intensidade_chuva': latest_messages[topic_intensidade_chuva],
        }
    })

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080, debug=True)