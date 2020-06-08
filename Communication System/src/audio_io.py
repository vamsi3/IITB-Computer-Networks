# This code is sourced (inspired) from and found at Github under the MIT License.
# All credits to the original authors. If you are the author, Thank You!

import math
import struct
import pyaudio as pa

def play_frequency(frequency, fs, stream, amplitude = 5, duration = 0.31102):
	N = int(fs / frequency)
	T = int(frequency * duration)  # repeat for T cycles
	dt = 1.0 / fs
	# 1 cycle
	tone = (amplitude * math.sin(2 * math.pi * frequency * n * dt)
			for n in range(N))
	# todo: get the format from the stream; this assumes Float32
	data = b''.join(struct.pack('f', samp) for samp in tone)
	for n in range(T):
		stream.write(data)



import wave
from sys import byteorder
from array import array
from scipy.fftpack import fft
import numpy as np

CHUNK = 1024
FORMAT = pa.paInt32
CHANNELS = 1
RATE = 48000

def record(audio, time_to_listen):
	stream = audio.open(format=FORMAT,
				channels=CHANNELS,
				rate=RATE,
				input=True,
				frames_per_buffer=CHUNK)

	# print("###RECORDING###")
	RECORD_SECONDS = time_to_listen

	s = array('h')
	frames = []

	for i in range(0, int(RATE / CHUNK * RECORD_SECONDS)):
		data = stream.read(CHUNK)
		frames.append(data)
		snd_data = array('h', data)
		if byteorder == 'big':
			snd_data.byteswap()
		s.extend(snd_data)

	# print("###RECORDING DONE###")

	stream.stop_stream()
	stream.close()
	audio.terminate()
	return s #, frames

def record_frequency(time_to_listen):
	X = fft(record(pa.PyAudio(), time_to_listen))
	freq = (np.abs(X)).argmax() / time_to_listen + 1
	return freq
