import os
import asyncio
import nest_asyncio
import logging
from telegram import Update
from telegram.ext import ApplicationBuilder, CommandHandler, ContextTypes
import paho.mqtt.client as mqtt
from dotenv import load_dotenv

load_dotenv()

logging.basicConfig(format='%(asctime)s - %(name)s - %(levelname)s - %(message)s', level=logging.INFO)

TELEGRAM_TOKEN = os.environ.get('TELEGRAM_BOT_TOKEN')

last_temperatura = None
last_umidade = None

async def send_telegram_message(context, chat_id, text):
    await context.bot.send_message(chat_id=chat_id, text=text)

def on_message(client, userdata, msg):
    global last_temperatura, last_umidade
    message = msg.payload.decode()

    if msg.topic == '/temperatura':
        last_temperatura = message
    elif msg.topic == '/umidade':
        last_umidade == message

async def start(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await update.message.reply_text('Olá! Estou pronto para fornecer dados de temperatura e umidade.')

    # Configuração do MQTT
    mqtt_client = mqtt.Client()
    mqtt_client.user_data_set({'context': context, 'chat_id': update.message.chat_id})
    mqtt_client.on_message = on_message

    # Conectar ao broker MQTT
    mqtt_client.connect('broker.emqx.io', 1883, 60)
    mqtt_client.subscribe('/temperatura')
    mqtt_client.subscribe('/umidade')

    mqtt_client.loop_start()