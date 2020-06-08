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

int main(int argc, char *argv[]) {
	if (argc != 4) {
		fprintf(stderr, "Use as: %s <ip-address:port-number> <file-to-receive> <receive-interval>\n", argv[0]);
		exit(1);
	}

	char* destination_ip_address = strtok(argv[1], ":");
	int destination_port_number = atoi(strtok(NULL, ":"));
	char* received_file_name = argv[2];
	int receive_interval = atoi(argv[3]);
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

	char file_name[5+strlen(received_file_name)+1] = {'g', 'e', 't', ' ', '\0'};
	strcat(file_name, received_file_name);
	write(socket_file_descriptor, file_name, 5+strlen(received_file_name)+1);

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
	close(socket_file_descriptor);
	return 0;
}
