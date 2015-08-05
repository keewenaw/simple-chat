#define closesocket close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>		// to get error numbers

#define	PROTOPORT	5193		/* default protocol port number */
#define	QLEN		6			/* size of request queue	*/

/*------------------------------------------------------------------------
 * Program:   server
 *
 * Purpose:   allocate a socket and then repeatedly execute the following:
 *		(1) wait for the next connection from a client
 *		(2) receive a short message from the client
 *		(3) display the message AND send the message back to the client
 *		(3) go back to step (2)
 *		(4) close connection after receiving "quit" signal
 *
 * Syntax:    server [ port ]
 *
 *		 port  - protocol port number to use
 *
 * Note:      The port argument is optional.  If no port is specified,
 *	      the server uses the default given by PROTOPORT.
 *
 *------------------------------------------------------------------------
 */

struct	protoent *ptrp;	 
struct	sockaddr_in sad; 
struct	sockaddr_in cad; 
int	sd, sd2;	 
int	port;		 
int	alen;		 
unsigned char	buf[1000];	 
int	n;
int	eof_flag=0;
int pid;

void chat(int socket)
{
	// Communication variables
	char escapeChar[] = "###"; // Sets what escape sequence is
	char breakStatement[strlen(escapeChar)]; // New array to test

	// Receive client's name
	int n;
	n = recv(socket, buf, sizeof(buf), 0); // Receive the name
	char name[strlen(buf)]; // Create a 'name' string of the right size
	strcpy(name, buf); // Copy the buffer into 'name' for future use
	write(1, name, n); // Write stuff to screen
	printf(" wants to chat (Socket #%d).\n", socket);
	memset(buf, 0, sizeof(buf)); // Clear the buffer
	
	// Send your own name
	printf("%d: What's your name? : ", socket); // Ask for name
	gets(buf); // Get input
	char name2[strlen(buf)]; // Store input
	strcpy(name2,buf);
	//buf[strlen(buf)] = '\n'; // Adds a line break to end of message
	send(socket, buf, strlen(buf), 0); // Put it in the buffer and send it
	//printf("\n\n"); // Make it pretty
	memset(buf, 0, sizeof(buf)); // Clear the buffer
	
	// Communicate until break triggered
	while(1)
	{
		int n;
		n = recv(socket, buf, sizeof(buf), 0); // Receive the message
		
		// Check if connection should be over
		//strncpy(breakStatement, buf, strlen(breakStatement)); // Copy the first few indices of buf to 'breakStatement'
		int i;
		for (i = 0; i < sizeof(breakStatement); i++) {
			breakStatement[i] = buf[i];
		}
		
		// If so, break
		// If first part of buffer is escape sequence,
		if ((breakStatement[0] == escapeChar[0]) && (breakStatement[1] == escapeChar[1]) && (breakStatement[2] == escapeChar[2])) {
			printf("\n\n%d: %s HAS LEFT THE CHAT.\n", socket, name); // Break the loop and call it a day
			exit(0);
		}
		
		// Else, continue
		else {
			memset(breakStatement, 0, sizeof(breakStatement)); // Clear 'breakStatement' for next time
			
			// Prints client's stuff to screen
			printf("\n\n%d: %s SAYS: \n", socket, name); // Print the name
			write(1, buf, n); // Print the text of the message
			printf("\n"); // Make it pretty
			
			memset(buf, 0, sizeof(buf)); // Clear the buffer
			
			// Say something back
			printf("\n%d: Say something back (or type '%s' to quit): ", socket, escapeChar); // Prompt
			gets(buf);
			
			// Prints your stuff to screen
			printf("\n%d: %s SAYS: \n", socket, name2); // Print your name
			write(1, buf, strlen(buf)); // Print the text of your message
			//printf("%s\n", buf); // Print the text of your message
			
			// Send it back and get ready for the next message
			//send(sd2, buf, strlen(buf), 0); // Send your message
			send(socket, buf, sizeof(buf), 0); // Send your message
			
			// Test if you wanted to break
			// Copy the first few indices of buf to 'breakStatement'
			for (i = 0; i < sizeof(breakStatement); i++) {
				breakStatement[i] = buf[i];
			}				
			
			// If so, break
			// If first part of buffer is escape sequence,
			if ((breakStatement[0] == escapeChar[0]) && (breakStatement[1] == escapeChar[1]) && (breakStatement[2] == escapeChar[2])) {
				printf("\n\n%d: YOU HAVE LEFT THE CHAT.\n\n", socket); // Break the loop and call it a day
				exit(0);
			}
			
			else { // If not, reset the buffers for the next message
				memset(breakStatement, 0, sizeof(breakStatement)); // Clear 'breakStatement' for next time
				memset(buf, 0, sizeof(buf)); // Clear the buffer
			}
		}
	}
}

int main(int argc, char **argv)
{	
	// clear the sad and cad structs
	memset((char *)&sad,0,sizeof(sad));
	memset((char *)&cad,0,sizeof(cad));
	 
	sad.sin_family = AF_INET;	  
	sad.sin_addr.s_addr = INADDR_ANY;   // note where place server address

	if (argc > 1) {			
		port = atoi(argv[1]);	
	} else {
		port = PROTOPORT;	
	}
	if (port > 0)			
		sad.sin_port = htons((u_short)port);	// note getting byte order correct
	else {				
		fprintf(stderr,"bad port number %s\n",argv[1]);
		exit(1);
	}
	
	// debug check
	printf("Port number is %d\n:", sad.sin_port);
	
	if ( ((int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "cannot map \"tcp\" to protocol number");
		exit(1);
	}

	// get a socket
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "socket creation failed\n");
		exit(1);
	}

	// do the bind
	if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		fprintf(stderr,"bind failed\n");
		exit(1);
	}

	// setup the queue
	if (listen(sd, QLEN) < 0) {
		fprintf(stderr,"listen failed\n");
		exit(1);
	}

/* start of loop referred to in step 6.2.10 from lab text*/

	while (1) {
		// Accept a connection, if no errors
		alen = sizeof(cad);
		if ( (sd2 = accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
			fprintf(stderr, "accept failed\n");
			exit(1);
		}

		if ((pid = fork()) == 0) {
			chat(sd2);
			exit(0);
		}
		
	//	if (wait(NULL) > 0) {
	//		printf("Server terminating ... \n");
	//		exit(0);
	//	}
	}
}
