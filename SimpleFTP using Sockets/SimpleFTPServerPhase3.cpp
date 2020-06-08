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

	if (listen(socket_file_descriptor, 10) < 0) {
		perror("listen");
		exit(9);
	} else {
		printf("ListenDone: %d\n", server_port_number);
	}

	fd_set master, read_fds, write_fds;
	FD_ZERO(&master); FD_ZERO(&read_fds); FD_ZERO(&write_fds);
	FD_SET(socket_file_descriptor, &master);
	int fdmax = socket_file_descriptor;

	int number_of_bytes;
	char buffer[BUFSIZ];
	int sin_size = sizeof(struct sockaddr);

	FILE* fd_to_file[1 << 18];

	while (true) {
		read_fds = master; write_fds = master;
		if (select(fdmax+1, &read_fds, &write_fds, NULL, NULL) < 0) {
			perror("select");
			exit(9);
		}

		for (int i=0; i<=fdmax; i++) {
			bzero(buffer, BUFSIZ);
			if (FD_ISSET(i, &read_fds)) {
				if (i == socket_file_descriptor) {
					struct sockaddr_in addr_of_client;
					int connection_socket_file_descriptor = accept(socket_file_descriptor, (struct sockaddr*) &addr_of_client, (socklen_t*) &sin_size);
					if (connection_socket_file_descriptor < 0) {
						perror("accept");
						continue;
					}
					printf("Client: %s:%d\n", inet_ntoa(addr_of_client.sin_addr), ntohs(addr_of_client.sin_port));
					if (connection_socket_file_descriptor > fdmax) {
						fdmax = connection_socket_file_descriptor;
					}
					FD_SET(connection_socket_file_descriptor, &master);
				} else if ((number_of_bytes = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
					close(i);
					if (fd_to_file[i]) fclose(fd_to_file[i]);
					fd_to_file[i] = NULL;
					FD_CLR(i, &master);
					continue;
				} else {
					char* file_name = buffer;

					char* send_file_name = strtok(file_name, " ");
					if (strcmp(send_file_name, "get")) {
						printf("UnknownCmd\n");
						fprintf(stderr, "Unknown filename command\n");
						close(i);
						if (fd_to_file[i]) fclose(fd_to_file[i]);
					fd_to_file[i] = NULL;
						FD_CLR(i, &master);
						continue;
					}
					send_file_name = strtok(NULL, "\0");
					strcat(send_file_name, "\0");
					printf("FileRequested: %s\n", send_file_name);

					FILE *send_file = fopen(send_file_name, "rb");
					if (send_file == NULL) {
						printf("FileTransferFail\n");
						fprintf(stderr, "File %s not found.\n", send_file_name);
						close(i);
						if (fd_to_file[i]) fclose(fd_to_file[i]);
					fd_to_file[i] = NULL;
						FD_CLR(i, &master);
						continue;
					} else {
						fd_to_file[i] = send_file;
					}
				}
			}
			if (FD_ISSET(i, &write_fds)) {
				if (!(fd_to_file[i])) continue;
				FILE* send_file = fd_to_file[i];

				char send_buffer[BUFSIZ];
				bzero(send_buffer, BUFSIZ);
				int send_file_size = 0, send_file_block_size;
				if ((send_file_block_size = fread(send_buffer, sizeof(char), BUFSIZ, send_file)) > 0) {
					int total_sent = 0, left_sent = send_file_block_size;
					while (total_sent < send_file_block_size) {
						int sent_size;
						if ((sent_size = send(i, send_buffer+total_sent, left_sent, 0)) < 0) {
							perror("send");
							close(i);
							if (fd_to_file[i]) fclose(fd_to_file[i]);
					fd_to_file[i] = NULL;
							FD_CLR(i, &master);
							continue;
						}
						total_sent += sent_size;
						left_sent -= sent_size;
					}
					send_file_size += send_file_block_size;
					bzero(send_buffer, BUFSIZ);
				} else if (send_file_block_size < 0) {
					printf("FileTransferFail\n");
					// fprintf(stderr, "File %s not readable.\n", send_file_name);
				} else {
					// printf("TransferDone: %d bytes\n", send_file_size);
					close(i);
					fclose(send_file);
					if (fd_to_file[i]) fclose(fd_to_file[i]);
					fd_to_file[i] = NULL;
					FD_CLR(i, &master);
					continue;
				}
			}
		}
	}
}
