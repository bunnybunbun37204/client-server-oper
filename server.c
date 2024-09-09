#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#define PORT 8080

// Function to send date data
void* sendDateData(void* socket_desc) {
	int new_socket = *(int*)socket_desc;
    	time_t current_time;
    	char* time_string;

    	free(socket_desc); // Free the allocated memory for socket descriptor

    	while (1) {
        	current_time = time(NULL);
        	time_string = ctime(&current_time);

        	// Check if sending data was successful
        	if (send(new_socket, time_string, strlen(time_string), 0) <= 0) {
            		perror("send failed");
            		break; // Exit loop if sending fails
        	}

        	sleep(1); // delay 1 sec
    }

    	close(new_socket);
    	return NULL;
}

int main() {
	
	// ใส่แล้ว server จะไม่ crash 	
	signal(SIGPIPE, SIG_IGN);

    	int server_fd, new_socket;
    	struct sockaddr_in address;
    	int opt = 1;
    	socklen_t addrlen = sizeof(address);

    	// Create Socket
    	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        	perror("socket failed");
        	exit(EXIT_FAILURE);
    	}

    	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        	perror("setsockopt");
        	exit(EXIT_FAILURE);
    	}

    	address.sin_family = AF_INET;
    	address.sin_addr.s_addr = INADDR_ANY;
    	address.sin_port = htons(PORT);

    // Bind Port
    	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        	perror("bind failed");
        	exit(EXIT_FAILURE);
    	}

    	if (listen(server_fd, 3) < 0) {
        	perror("listen");
        	exit(EXIT_FAILURE);
    	}

    	printf("Server listening on port %d\n", PORT);

    	while (1) {
        	if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
            		perror("accept");
            		continue; // Skip to the next iteration if accept fails
        	}

        	printf("New client connected\n");

        	int* new_sock = malloc(sizeof(int));
        	if (new_sock == NULL) {
            		perror("malloc failed");
            		close(new_socket);
            		continue; // Skip to the next iteration if malloc fails
        	}

        	*new_sock = new_socket;

        	pthread_t client_thread;
        	if (pthread_create(&client_thread, NULL, sendDateData, (void*)new_sock) != 0) {
            		perror("could not create thread");
            		free(new_sock); // Free memory if thread creation fails
            		close(new_socket);
            		continue; // Skip to the next iteration if thread creation fails
        	}

        	pthread_detach(client_thread); // Detach thread to avoid hanging
    	}

    	close(server_fd);

    	return 0;
}
