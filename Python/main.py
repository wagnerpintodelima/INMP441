import serial
import pyaudio
import numpy as np

# Buffer global para armazenar os dados
audio_buffer = []

# Configurações do UART
uart_port = 'COM2'  # Substitua pela sua porta serial
baud_rate = 250000

# Configurações do PyAudio
CHUNK = 8000  # Número de amostras por vez
FORMAT = pyaudio.paInt16  # Formato de 16 bits
CHANNELS = 1               # Mono
RATE = 8000                # Taxa de amostragem

# Abre a porta serial
ser = serial.Serial(uart_port, baud_rate)

p = pyaudio.PyAudio()
stream = p.open(format=FORMAT, channels=CHANNELS, rate=RATE, output=True)

# GAIN
GAIN = 10

while True:
    line = ser.readline()  # Lê até o caractere de nova linha
    
    try:
        # Converte a linha em um inteiro
        sample = int(line.strip())  # Remove espaços em branco e converte para int
        print(sample)
        sample *= GAIN
        
        audio_buffer.append(sample)  # Alimentar o buffer com as amostras
            
    except ValueError:
        #print("Erro ao converter linha:", line)  # Trate erros de conversão
        pass
        

    if len(audio_buffer) >= CHUNK:
        chunk = audio_buffer[:CHUNK]
        del audio_buffer[:CHUNK]  # Remove o chunk processado do buffer
    else:
        chunk = []  # Inicializa chunk como vazio se não houver amostras suficientes

    if chunk:  # Verifica se o chunk não está vazio
        chunk_np = np.array(chunk, dtype=np.int16)
        stream.write(chunk_np.tobytes())