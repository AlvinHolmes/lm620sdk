import numpy as np
import wave
import struct

# 参数设置
sample_rate = 8000  # 采样率
frequency = 440  # 频率
duration = 3  # 持续时间（秒）
channels = 1  # 声道数，1为单声道，2为双声道
amplitude = 0.8
bit_width = 16

# 生成时间序列
t = np.linspace(0, duration, int(sample_rate * duration), endpoint=False)

# 生成正弦波
sine_wave = amplitude * np.sin(2 * np.pi * frequency * t)

# 根据声道数扩展正弦波
if channels == 2:
    audio_wave = np.column_stack((sine_wave, sine_wave))
else:
    audio_wave = sine_wave

# 将数据转换为24位整数
max_amplitude = 2**(bit_width - 1) - 1
audio_data = (audio_wave * max_amplitude)

if bit_width == 16:
    audio_data = audio_data.astype(np.int16)
elif bit_width == 24 or bit_width == 32:
    audio_data = audio_data.astype(np.int32)

# 保存为WAV文件
with wave.open('.backup/audio_data.wav', 'w') as wav_file:
    wav_file.setnchannels(channels)
    wav_file.setsampwidth(bit_width // 8)
    wav_file.setframerate(sample_rate)
    for sample in audio_data:
        if channels == 2:
            if bit_width == 16:
                wav_file.writeframes(struct.pack('<h', sample[0]) + struct.pack('<h', sample[1]))
            elif bit_width == 24:
                wav_file.writeframes(struct.pack('<i', sample[0])[:3] + struct.pack('<i', sample[1])[:3])
            elif bit_width == 32:
                wav_file.writeframes(struct.pack('<i', sample[0]) + struct.pack('<i', sample[1]))
        else:
            if bit_width == 16:
                wav_file.writeframes(struct.pack('<h', sample))
            elif bit_width == 24:
                wav_file.writeframes(struct.pack('<i', sample)[:3])
            elif bit_width == 32:
                wav_file.writeframes(struct.pack('<i', sample))

# 保存为CSV文件，格式适合C数组
with open('audio_data.csv', 'w') as csv_file:
    for sample in audio_data:
        if channels == 2:
            csv_file.write(f'{sample[0]}, {sample[1]},\n')
        else:
            csv_file.write(f'{sample},\n')

print("WAV文件和CSV文件已生成。")
