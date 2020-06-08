from math import floor, log2

def hamming_encode(M):
	inp = list(map(int, list(M)))
	inp += [1]
	while (len(inp) != 21):
		inp += [0]


	len_enc = len(inp) + (1+floor(log2(len(inp))))
	while (len_enc - (1+floor(log2(len_enc))) != len(inp)):
		len_enc += 1
	len_enc += 1
	parity_cnt = len_enc - len(inp)
	enc = [0]*len_enc
	iter = 0
	for i in range(1, len_enc):
		if (i & (i-1) != 0):
			enc[i] = inp[iter]
			enc[0] ^= inp[iter]
			iter += 1
	for i in range(parity_cnt-1):
		pos = 2**i
		for j in range(pos+1, len_enc):
			if ((j >> i) & 1):
				enc[pos] ^= enc[j]
		enc[0] ^= enc[pos]
	return ''.join(str(x) for x in enc)

def hamming_decode(enc):
	inp = list(map(int, list(enc)))
	len_inp = len(inp)
	# print(len_inp)
	# print(inp)
	for i in range(1, len_inp):
		inp[0] ^= inp[i]
	pos = 1
	parity_cnt = 1
	error_correction = 0
	no_of_errors = 0
	while (pos < len_inp):
		temp = inp[pos]
		for j in range(pos+1, len_inp):
			if ((j >> (parity_cnt-1)) & 1):
				temp ^= inp[j]
		if (temp != 0):
			error_correction += pos
		pos *= 2
		parity_cnt += 1
		# print(pos, len_inp)
	len_msg = len_inp-parity_cnt
	M = [0]*len_msg
	iter = 0
	one_error_pos = -1
	for i in range(1, len_inp):
		if (i & (i-1) != 0):
			if (i == error_correction):
				one_error_pos = iter
				M[iter] = 1-inp[i]
			else:
				M[iter] = inp[i]
			iter += 1
	if (inp[0] == 0 and error_correction != 0):
		return 'retransmit'

	i = len(M)-1
	while (M[i] != 1):
		i -= 1
	M = M[:i]
	M = ''.join(str(x) for x in M)
	if (error_correction != 0):
		print("Error found only at index {} in encoded message. Hence corrected".format(error_correction))
		if (one_error_pos != -1 and one_error_pos < len(M)):
			print("And, the corresponding position in message is {}".format(one_error_pos+1))
	return M
