#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

void SendFileToServer(char* send_file_name, int socket_file_descriptor) {
	FILE *send_file = fopen(send_file_name, "rb");
	if (send_file == NULL) {
		printf("FileTransferFail\n");
		fprintf(stderr, "File %s not found.\n", send_file_name);
		return;
	}

	char send_buffer[BUFSIZ];
	bzero(send_buffer, BUFSIZ);
	int send_file_size = 0, send_file_block_size;
	while ((send_file_block_size = fread(send_buffer, sizeof(char), BUFSIZ, send_file)) > 0) {
		int total_sent = 0, left_sent = send_file_block_size;
		while (total_sent < send_file_block_size) {
			int sent_size;
			if ((sent_size = send(socket_file_descriptor, send_buffer+total_sent, left_sent, 0)) < 0) {
				perror("send");
				exit(9);
			}
			total_sent += sent_size;
			left_sent -= sent_size;
		}
		send_file_size += send_file_block_size;
		bzero(send_buffer, BUFSIZ);
	}
	if (send_file_block_size < 0) {
		printf("FileTransferFail\n");
		fprintf(stderr, "File %s not readable.\n", send_file_name);
		return;
	} else {
		printf("TransferDone: %d bytes\n", send_file_size);
	}
	fclose(send_file);
}

int main(int argc, char *argv[]) {
	if (argc != 5) {
		fprintf(stderr, "Use as: %s <ip-address:port-number> <operator> <file-to-receive> <receive-interval>\n", argv[0]);
		exit(1);
	}

	char* destination_ip_address = strtok(argv[1], ":");
	int destination_port_number = atoi(strtok(NULL, ":"));
	char* op = argv[2];
	char* received_file_name = argv[3];
	int receive_interval = atoi(argv[4]);
	int socket_file_descriptor;
	if ((socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(9);
	}

	struct sockaddr_in addr_of_destination;
	addr_of_destination.sin_family = AF_INET;
	addr_of_destination.sin_port = htons(destination_port_number);
	addr_of_destination.sin_addr.s_addr = inet_addr(destination_ip_address);
	bzero(&(addr_of_destination.sin_zero), 8);

	if (connect(socket_file_descriptor, (struct sockaddr*) &addr_of_destination, (socklen_t) sizeof(struct sockaddr)) < 0) {
		perror("connect");
		exit(2);
	} else {
		printf("ConnectDone: %s:%d\n", destination_ip_address, destination_port_number);
	}

	char file_name[4+strlen(received_file_name)+1];
	strcpy(file_name, op);
	strcat(file_name, " ");
	strcat(file_name, received_file_name);

	write(socket_file_descriptor, file_name, strlen(file_name)+1);

	if (strcmp(op, "get") == 0) {
		char file_data_buffer[BUFSIZ];
		FILE* received_file = fopen(received_file_name, "wb");
		if (received_file == NULL) {
			fprintf(stderr, "Couldn't create file %s\n", received_file_name);
			exit(3);
		}
		int received_file_size = 0, received_file_block_size;
		while ((received_file_block_size = recv(socket_file_descriptor, file_data_buffer, 1000, 0)) > 0) {
			fwrite(file_data_buffer, sizeof(char), received_file_block_size, received_file);
			bzero(file_data_buffer, BUFSIZ);
			received_file_size += received_file_block_size;
			usleep(receive_interval*1000);
		}
		if (received_file_block_size < 0) {
			fprintf(stderr, "File %s not writeable.\n", received_file_name);
			exit(3);
		} else {
			printf("FileWritten: %d bytes\n", received_file_size);
		}

		fclose(received_file);

	} else if (strcmp(op, "put") == 0) {
		SendFileToServer(received_file_name, socket_file_descriptor);
	} else {
		fprintf(stderr, "Error in command operation\n");
		exit(9);
	}

	close(socket_file_descriptor);
	return 0;
}
