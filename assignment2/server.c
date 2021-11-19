// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#include <pwd.h> // Addition for Assignment 1

#define PORT 8080

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";

    char *nobodyuser = "nobody"; // Addition for Assignment 1
    struct passwd* nobodypwd; // Addition for Assignment 1
    pid_t pid; // Addition for Assignment 1
    
    if (strcmp(argv[0], "child") == 0) //if the filename comparison is the same then execvp is already performed 
    {
        int another_new_socket = atoi(argv[1]); //convert to int and read the parent socket address
        valread = read(another_new_socket, buffer, 1024);
        
        printf("Read %d bytes: %s\n", valread, buffer);
        
        send(another_new_socket, hello, strlen(hello), 0); //send the response to the client
        printf("Hello message sent\n");
        exit(0);
    }
    
    else
    {

	    // Creating socket file descriptor
	    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	    {
		perror("socket failed");
		exit(EXIT_FAILURE);
	    }

	    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	    {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	    }
	    address.sin_family = AF_INET;
	    address.sin_addr.s_addr = INADDR_ANY;
	    address.sin_port = htons( PORT );

	    if (bind(server_fd, (struct sockaddr *)&address,
		sizeof(address)) < 0)
	    {
		perror("bind failed");
		exit(EXIT_FAILURE);
	    }

	    if (listen(server_fd, 3) < 0)
	    {
		perror("listen");
		exit(EXIT_FAILURE);
	    }

	    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
		               (socklen_t*)&addrlen))<0)
	    {
		perror("accept");
		exit(EXIT_FAILURE);
	    }

	    //------seperate privs here--------//

	    pid = fork(); 
	    if (pid == 0) //if in child process
	    {
	    	int dup_parent_socket = dup(new_socket); //duplicate new socket file descriptor
	    	
		if (dup_parent_socket == -1)
		{
			perror("Duplication failiure!");
			exit(EXIT_FAILURE);
		}
		
		if ((nobodypwd = getpwnam(nobodyuser)) == NULL) //find nobody user
		{
		    perror("error finding Nobody User!");
		    exit(EXIT_FAILURE);
		}
		setuid(nobodypwd->pw_uid); //change priv of child process
		
	      	//get ready to pass the parent socket address as args
		char parent_socket_address_char[10];
		sprintf(parent_socket_address_char, "%d", dup_parent_socket);
		char *args[] = {"child", parent_socket_address_char, NULL}; 
		
		execvp(argv[0], args); // perform execvp and pass the parent socket addresses
	    }
	    wait();
    }
	
    return 0;
}
