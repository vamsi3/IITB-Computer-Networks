#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<signal.h>

void SendFileToClient(char* send_file_name, int connection_socket_file_descriptor) {
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
			if ((sent_size = send(connection_socket_file_descriptor, send_buffer+total_sent, left_sent, 0)) < 0) {
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
	if (argc != 2) {
		fprintf(stderr, "Use as: %s <port-number>\n", argv[0]);
		exit(1);
	}

	int server_port_number = atoi(argv[1]);
	int socket_file_descriptor;
	if ((socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(9);
	}

	struct sockaddr_in addr_of_server;
	addr_of_server.sin_family = AF_INET;
	addr_of_server.sin_port = htons(server_port_number);
	addr_of_server.sin_addr.s_addr = INADDR_ANY;
	bzero(&(addr_of_server.sin_zero), 8);
	printf("Address of server: %s\n", inet_ntoa(addr_of_server.sin_addr));
	
	if (bind(socket_file_descriptor, (struct sockaddr*) &addr_of_server, sizeof(struct sockaddr)) < 0) {
		perror("bind");
		exit(2);
	} else {
		printf("BindDone: %d\n", server_port_number);
	}

	if (listen(socket_file_descriptor, 1) < 0) {
		perror("listen");
		exit(9);
	} else {
		printf("ListenDone: %d\n", server_port_number);
	}

	struct sockaddr_in addr_of_client;
	int sin_size = sizeof(struct sockaddr);
	while (true) {
		int connection_socket_file_descriptor = accept(socket_file_descriptor, (struct sockaddr*) &addr_of_client, (socklen_t*) &sin_size);
		if (connection_socket_file_descriptor < 0) {
			perror("accept");
			exit(9);
		}
		printf("Client: %s:%d\n", inet_ntoa(addr_of_client.sin_addr), ntohs(addr_of_client.sin_port));

		char file_name[BUFSIZ];
		int file_name_size;
		if ((file_name_size = read(connection_socket_file_descriptor, file_name, BUFSIZ)) < 0) {
			fprintf(stderr, "Couldn't get filename\n");
			exit(9);
		}

		char* send_file_name = strtok(file_name, " ");
		if (strcmp(send_file_name, "get")) {
			printf("UnknownCmd\n");
			fprintf(stderr, "Unknown filename command\n");
			close(connection_socket_file_descriptor);
			continue;
		}
		send_file_name = strtok(NULL, "\0");
		strcat(send_file_name, "\0");
		printf("FileRequested: %s\n", send_file_name);

		SendFileToClient(send_file_name, connection_socket_file_descriptor);
		close(connection_socket_file_descriptor);
	}

	// close(socket_file_descriptor);
	// return 0;
}
