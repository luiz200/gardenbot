import os
import asyncio
import nest_asyncio
import logging
from telegram import Update
from telegram.ext import ApplicationBuilder, CommandHandler, ContextTypes, ConversationHandler, MessageHandler, filters
import paho.mqtt.client as mqtt
from dotenv import load_dotenv

load_dotenv()

nest_asyncio.apply()

logging.basicConfig(format='%(asctime)s - %(name)s - %(levelname)s - %(message)s', level=logging.INFO)

TELEGRAM_TOKEN = os.environ.get('TELEGRAM_BOT_TOKEN')

last_temperatura = None
last_umidade = None

USERNAME, PASSWORD = range(2)

def on_message(client, userdata, msg):
    global last_temperatura, last_umidade
    message = msg.payload.decode()

    #logging.info(f"Mensagem recebida no tópico {msg.topic}: {message}")

    if msg.topic == '/temperatura':
        last_temperatura = message
    elif msg.topic == '/umidade_ar':
        last_umidade = message

def connect_mqtt(username, password):
    client = mqtt.Client()
    client.on_message = on_message
    client.username_pw_set(username, password)

    try:
        client.connect('broker.emqx.io', 1883, 60)
        client.subscribe('/temperatura')
        client.subscribe('/umidade_ar')
        client.loop_start()
        return client
    except Exception as e:
        logging.error(f"Erro ao conectar ao MQTT: {e}")
        return None
    
async def start(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    await update.message.reply_text('Olá! Por favor, forneça seu nome de usuário MQTT:')
    return USERNAME

async def get_username(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    context.user_data['MQTT_USERNAME'] = update.message.text
    await update.message.reply_text('Nome de usuário MQTT recebido. Agora, forneça sua senha MQTT:')
    return PASSWORD

async def get_password(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    context.user_data['MQTT_PASSWORD'] = update.message.text
    await update.message.reply_text('Senha MQTT recebida. Tentando conectar...')

    mqtt_client = connect_mqtt(context.user_data['MQTT_USERNAME'], context.user_data['MQTT_PASSWORD'])
    if mqtt_client:
        context.user_data['mqtt_client'] = mqtt_client
        await update.message.reply_text('Conectado ao MQTT com sucesso. Você pode começar a usar os comandos /temperatura e /umidade.')
        return ConversationHandler.END
    else:
        await update.message.reply_text('Falha ao conectar ao MQTT. Verifique suas credenciais e tente novamente.')
        return ConversationHandler.END

async def get_temperatura(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    if last_temperatura is not None:
        await update.message.reply_text(f'A temperatura atual é {last_temperatura}°C')
    else:
        await update.message.reply_text('Os dados de temperatura ainda não foram recebidos.')

async def get_umidade(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    if last_umidade is not None:
        await update.message.reply_text(f'A umidade atual é {last_umidade}%')
    else:
        await update.message.reply_text('Os dados de umidade não foram recebidos.')

async def main():
    application = ApplicationBuilder().token(TELEGRAM_TOKEN).build()

    conv_handler = ConversationHandler(
        entry_points=[CommandHandler('start', start)],
        states={
            USERNAME: [MessageHandler(filters.TEXT & ~filters.COMMAND, get_username)],
            PASSWORD: [MessageHandler(filters.TEXT & ~filters.COMMAND, get_password)],
        },
        fallbacks=[],
    )

    application.add_handler(conv_handler)
    application.add_handler(CommandHandler("temperatura", get_temperatura))
    application.add_handler(CommandHandler("umidade", get_umidade))

    await application.run_polling()

if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    loop.run_until_complete(main())