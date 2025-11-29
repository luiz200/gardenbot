from datetime import datetime
import os
import csv
from flask import Flask, request, jsonify, render_template, send_file
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
    topic_chuva: 0,
    topic_intensidade_chuva: 0,
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

def get_csv_file():
    os.makedirs("csv", exist_ok=True)
    today = datetime.now().strftime("%Y-%m-%d")
    filename = f"csv/dados_{today}.csv"

    if not os.path.exists(filename):
        with open(filename, mode='w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow([
                "timestamp",
                "temperatura",
                "umidade_ar",
                "umidade_solo1",
                "umidade_solo2",
                "umidade_solo3",
                "umidade_solo4",
                "chuva",
                "intensidade_chuva"
            ])
    return filename

@mqtt_client.on_message()
def handle_mqtt_message(client, userdata, message):
    global latest_messages

    data = message.payload.decode()
    topic = message.topic

    latest_messages[topic] = data

    filename = get_csv_file()

    row = [
        datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        latest_messages[topic_temperature],
        latest_messages[topic_umidade_ar],
        latest_messages[topic_umidade_solo1],
        latest_messages[topic_umidade_solo2],
        latest_messages[topic_umidade_solo3],
        latest_messages[topic_umidade_solo4],
        latest_messages[topic_chuva],
        latest_messages[topic_intensidade_chuva],
    ]

    with open(filename, mode='a', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow(row)


@app.route('/')
def index():
    return render_template('index.html',
                           latest_temperature=latest_messages[topic_temperature],
                           latest_umidade_ar=latest_messages[topic_umidade_ar],
                           latest_umidade_solo1=latest_messages[topic_umidade_solo1],
                           latest_umidade_solo2=latest_messages[topic_umidade_solo2],
                           latest_umidade_solo3=latest_messages[topic_umidade_solo3],
                           latest_umidade_solo4=latest_messages[topic_umidade_solo4],
                           latest_chuva=latest_messages[topic_chuva],
                           lastest_intensidade=latest_messages[topic_intensidade_chuva],
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

@app.route('/listar_csv')
def listar_csv():
    folder = "csv"
    os.makedirs(folder, exist_ok=True)

    arquivos = []

    for nome in os.listdir(folder):
        if nome.endswith(".csv"):
            data = nome.replace("dados_", "").replace(".csv", "")
            arquivos.append({
                "nome": nome,
                "data": data
            })

    arquivos.sort(key=lambda x: x["data"], reverse=True)

    return render_template("listar_csv.html", arquivos=arquivos)

@app.route('/download_csv_by_date')
def download_csv_by_date():
    date_str = request.args.get("data")

    if not date_str:
        return "Data não fornecida", 400

    filename = f"csv/dados_{date_str}.csv"

    if not os.path.exists(filename):
        return f"Arquivo {filename} não encontrado.", 404

    return send_file(
        filename,
        mimetype="text/csv",
        as_attachment=True,
        download_name=os.path.basename(filename)
    )

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080, debug=True)