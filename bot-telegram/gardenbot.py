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
last_umidadesolo1 = None
last_umidadesolo2 = None
last_umidadesolo3 = None
last_umidadesolo4 = None
last_chuva = 0
last_intensidade_chuva = 0

USERNAME, PASSWORD = range(2)

def on_message(client, userdata, msg):
    global last_temperatura, last_umidade, last_umidadesolo1, last_umidadesolo2, last_umidadesolo3, last_umidadesolo4, last_chuva, last_intensidade_chuva
    message = msg.payload.decode()

    #logging.info(f"Mensagem recebida no tópico {msg.topic}: {message}")

    if msg.topic == '/temperatura':
        last_temperatura = message
    elif msg.topic == '/umidade_ar':
        last_umidade = message
    elif msg.topic == '/umidade_solo1':
        last_umidadesolo1 = message
    elif msg.topic == '/umidade_solo2':
        last_umidadesolo2 = message
    elif msg.topic == '/umidade_solo3':
        last_umidadesolo3 = message
    elif msg.topic == '/umidade_solo4':
        last_umidadesolo4 = message
    elif msg.topic == '/chuva':
        last_chuva == int(message)
    elif msg.topic == '/intensidade_chuva':
        last_intensidade_chuva == int(message)
    

def connect_mqtt(username, password):
    client = mqtt.Client()
    client.on_message = on_message
    client.username_pw_set(username, password)

    try:
        client.connect('broker.emqx.io', 1883, 60)
        client.subscribe('/temperatura')
        client.subscribe('/umidade_ar')
        client.subscribe('/umidade_solo1')
        client.subscribe('/umidade_solo2')
        client.subscribe('/umidade_solo3')
        client.subscribe('/umidade_solo4')
        client.subscribe('/chuva')
        client.subscribe('/intensidade_chuva')
        client.loop_start()
        return client
    except Exception as e:
        logging.error(f"Erro ao conectar ao MQTT: {e}")
        return None
    
def intensidade():
    if last_intensidade_chuva < 1000:
        return 'Chuva intensa'
    elif last_intensidade_chuva <= 3000 or last_intensidade_chuva >= 1000:
        return 'Chuva Moderada ou Chuvisco'
    elif last_intensidade_chuva > 4000:
        return 'Sem previsão de chuva'

def chuva():
    if last_chuva == 1:
        return 'Sim'
    elif last_chuva == 0:
        return 'Não'
    
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
        await update.message.reply_text('Conectado ao MQTT com sucesso. Você pode começar a usar os comandos.')
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

async def get_chuva(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    if last_chuva is not None:
        await update.message.reply_text(f'Está chovendo? {last_chuva}')
    else:
        await update.message.reply_text('Os dados de umidade não foram recebidos.')

async def get_intensidade(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    if last_intensidade_chuva is not None:
        inte = intensidade()
        await update.message.reply_text(f'Intensidade: {inte}')
    else:
        await update.message.reply_text('Os dados de umidade não foram recebidos.')

async def get_umidadesolo1(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    if last_umidadesolo1 is not None:
        await update.message.reply_text(f'A umidade do vaso-1 atual é {last_umidadesolo1}%')
    else:
        await update.message.reply_text('Os dados de umidade do vaso-1 não foram recebidos.')

async def get_umidadesolo2(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    if last_umidadesolo2 is not None:
        await update.message.reply_text(f'A umidade do vaso-2 atual é {last_umidadesolo2}%')
    else:
        await update.message.reply_text('Os dados de umidade do vaso-2 não foram recebidos.')

async def get_umidadesolo3(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    if last_umidadesolo3 is not None:
        await update.message.reply_text(f'A umidade do vaso-3 atual é {last_umidadesolo3}%')
    else:
        await update.message.reply_text('Os dados de umidade do vaso-3 não foram recebidos.')

async def get_umidadesolo4(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    if last_umidadesolo4 is not None:
        await update.message.reply_text(f'A umidade do vaso-4 atual é {last_umidadesolo4}%')
    else:
        await update.message.reply_text('Os dados de umidade do vaso-4 não foram recebidos.')

async def get_status(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    if last_temperatura is not None and last_umidade is not None and last_umidadesolo1 is not None and last_umidadesolo2 is not None and last_umidadesolo3 is not None and last_umidadesolo4 is not None:
        intensidade_chuvas = intensidade()
        chuvas = chuva()
        status_message = (
            f'Dados do ambiente:\n'
            f'\n'
            f'Temperatura:  {last_temperatura}° | Umidade:  {last_umidade}%\n'
            f'Chuva:  {chuvas}° | Instensidade:  {intensidade_chuvas}%\n'
            f'\n'
            f'Dados da plantação:\n'
            f'\n'
            f'Vaso-1:  {last_umidadesolo1}% | Vaso-2:  {last_umidadesolo2}%\n'
            f'Vaso-3:  {last_umidadesolo2}% | Vaso-4:  {last_umidadesolo4}%'
        )
        await update.message.reply_text(status_message)
    else:
        await update.message.reply_text('Os dados ainda não foram recebidos.')

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
    application.add_handler(CommandHandler("chuva", get_chuva))
    application.add_handler(CommandHandler("intensidade", get_intensidade))
    application.add_handler(CommandHandler("umidadesolo1", get_umidadesolo1))
    application.add_handler(CommandHandler("umidadesolo2", get_umidadesolo2))
    application.add_handler(CommandHandler("umidadesolo3", get_umidadesolo3))
    application.add_handler(CommandHandler("umidadesolo4", get_umidadesolo4))
    application.add_handler(CommandHandler("status", get_status))

    await application.run_polling()

if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    loop.run_until_complete(main())