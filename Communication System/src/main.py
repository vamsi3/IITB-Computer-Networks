from generate_listen import host_generate, host_listen, client_generate, client_listen
from hamming_coding import hamming_encode, hamming_decode
from math import floor, log2

# parameters to tweak
amplitude = 6
time_to_listen = 0.045

def msg_pos(n):
	ans = n + (1+floor(log2(n)))
	while (ans - (1+floor(log2(ans))) != n):
		ans += 1
	return ans


# host module
def host(list_of_messages, self_error = []):
	message_index = 0
	for message in list_of_messages:
		encoded_message = hamming_encode(message)
		encoded_message = list(map(int, list(encoded_message))) # converting string to list
		if (message_index < len(self_error)):
			for position in self_error[message_index]:
				encoded_message[position] = 1-encoded_message[position]
		encoded_message = ''.join(str(x) for x in encoded_message) # converting list back to string
		print("The encoded message is:", encoded_message)
		host_generate(encoded_message) # sending the encoded message
		response = host_listen(time_to_listen) # waiting for acknowledgement or retransmit response
		while (response == 0):
			print("Retransmission request recieved from client... sending again")
			host_generate(hamming_encode(message))
			response = host_listen(time_to_listen)
		message_index += 1

# client module
def client():
	message_index = 0
	while(True):
		encoded_message = client_listen(time_to_listen) # listening for a message from client
		message = hamming_decode(encoded_message)
		while (message == 'retransmit'):
			print("Requesting for retranmission to host...")
			client_generate(0) # making a retransmit request
			encoded_message = client_listen(time_to_listen)
			message = hamming_decode(encoded_message)
		print("Sending successful acknowledgement...")
		client_generate(1) # sending an acknowledgement for success
		print("Message {} recieved is {}".format(message_index, message))
		message_index += 1
