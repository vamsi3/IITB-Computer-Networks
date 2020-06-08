from audio_io import play_frequency, record_frequency
from time import sleep

freq_val = {0: 1500, 1: 8000, 2: 4200}

import pyaudio as pa

fs = 48000
p = pa.PyAudio()
stream = p.open(
	format=pa.paFloat32,
	channels=1,
	rate=fs,
	output=True)

def getval(f):
	ans = 0
	dis = abs(f-freq_val[0])
	if (abs(f-freq_val[1]) < dis):
		dis = abs(f-freq_val[1])
		ans = 1
	if (abs(f-freq_val[2]) < dis):
		dis = abs(f-freq_val[2])
		ans = 2
	if (abs(f) < dis):
		dis = abs(f-freq_val[2])
		ans = 3
	return ans

def host_listen(time_to_listen = 0.3):
	sleep(0.4)
	x = getval(record_frequency(time_to_listen))
	while (x == 2 or x == 3):
		x = getval(record_frequency(time_to_listen))
		# print(x)
	return x

def host_generate(M):
	sleep(2)
	enc = list(map(int, list(M)))
	len_enc = len(enc)
	prev = enc[0]
	play_frequency(freq_val[enc[0]], fs, stream)
	# print(enc[0], freq_val[enc[0]])
	cnt = 1
	for i in range(1, len_enc):
		# print(enc[i], end=' ')
		if (enc[i] == prev):
			cnt += 1
			if (cnt%2 == 0):
				play_frequency(freq_val[2], fs, stream)
				# print(freq_val[2])
			else:
				play_frequency(freq_val[enc[i]], fs, stream)
				# print(freq_val[enc[i]])
		else:
			cnt = 1
			play_frequency(freq_val[enc[i]], fs, stream)
			# print(freq_val[enc[i]])
		prev = enc[i]

def client_listen(time_to_listen = 0.3):
	sleep(1)
	curr_f = record_frequency(time_to_listen)
	enc = ""
	f = 3
	while (len(enc) < 27):
		x_freq = record_frequency(time_to_listen)
		if (x_freq < 800 or x_freq > 10000):
			# print("/////////", x_freq)
			continue
		curr_f = getval(x_freq)
		print(len(enc), curr_f, enc, x_freq)
		if (f == curr_f):
			continue
		else:
			if (curr_f == 2):
				enc += str(enc[len(enc)-1])
			elif (curr_f == 0 or curr_f == 1):
				enc += str(curr_f)
			# if (len(enc) == 5):
			# 	# print(enc, "----------------------------------------", f, curr_f, x_freq)
			# 	length += int(enc, 2)
		f = curr_f
	print("Encoded message received is:", enc)
	return enc

def client_generate(i):
	sleep(1)
	# print("Sending ack!")
	play_frequency(freq_val[i], fs, stream)
	play_frequency(freq_val[i], fs, stream)
