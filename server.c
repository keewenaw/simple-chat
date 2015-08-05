#define closesocket close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define	PROTOPORT	5193		

extern	int		errno;	

/*------------------------------------------------------------------------
* Program:  client
*
* Purpose:  allocate a socket and then repeatedly execute the following:
*		(1) connect to server using IP/port numbers from command line
*		(2) get a string from the user - put into buffer
*		(3) display the message AND send the message back to the server
*		(4) receiver message from server AND display the message
*		(5) go back to step (2)
*		(4) close connection after sending "quit" signal
*
* Syntax:  client server_IP_addr [server_port]
*
* Note:   The port argument is optional. If no port is specified,
*	   the client uses the default given by PROTOPORT.
*
*------------------------------------------------------------------------
*/

int main(int argc, char **argv)
{
	struct	hostent	*ptrh;	
	struct	protoent *ptrp;	
	struct	sockaddr_in sad;
	struct	sockaddr_in cad;
	int	sd,sd2;	
		int	alen;	
	int	port;		
	char	*host;		
	int	n;		
	unsigned char	buf[1000];
	char escapeChar[] = "###"; // Sets what escape sequence is
	char breakStatement[strlen(escapeChar)]; // New array to test
	
	// clear the sad struct
	memset((char *)&sad,0,sizeof(sad));

	sad.sin_family = AF_INET;	

	if (argc > 2) {			
		port = atoi(argv[2]);
		printf("port : %d \n",port);
	} else {
		port = PROTOPORT;	
	}
	if (port > 0)			
		sad.sin_port = htons((u_short)port); // get bytes in order
	else {				
		fprintf(stderr,"bad port number %s\n",argv[2]);
		exit(1);
	}

	if (argc > 1) {
		host = argv[1];
	}
 
	ptrh = gethostbyname(host);
	if ( ((char *)ptrh) == NULL ) {
		printf("Host Has been looked up!\n");
		fprintf(stderr,"invalid host: %s\n", host);
		printf("Host Has been looked up and it is : %s\n!",host);
		exit(1);
	}
	memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);
 
	if ( ((int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "cannot map \"tcp\" to protocol number");
		exit(1);
	}
 
	// create a socket
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "socket creation failed\n");
		exit(1);
	}
 
	// connect to server
	if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		fprintf(stderr,"connect failed\n");
		exit(1);
	}
 
  /* using gets will give you a warning with gcc. */
 
		while (1) {
     
      // Send your own name
      printf("What's your name? : "); // Ask for name
			
      gets(buf); // Input your name
			
      char name2[strlen(buf)]; // Stores your name
			
      strcpy(name2,buf); // Copys name for future use
			
      send(sd, buf, strlen(buf), 0); // Put it in the buffer and send it
			
      memset(buf, 0, sizeof(buf)); // Clear the buffer
			
      // Receive client's name
      int n;
      n = recv(sd, buf, sizeof(buf), 0); // Receive the name
			
      char name[strlen(buf)]; // Create a 'name' string of the right size
			
      strcpy(name, buf); // Copy the buffer into 'name' for future use
			
      write(1, name, n); // Print name to screen
			
      printf(" wants to chat.\n"); // Text
			
      memset(buf, 0, sizeof(buf)); // Clear the buffer
			
			// Say something back
			printf("Say something back (or type '%s' to quit): ", escapeChar); // Prompt
			
			gets(buf); // Accepts your text
			
			// Prints your stuff to screen
			
			printf("%s says: \n", name2); // Prints your name
			
			write(1, buf, n); // Print the text of your message
			
			printf("\n"); // Make it pretty	
			
			// Send it back and get ready for the next message
			
			send(sd, buf, strlen(buf), 0); // Send your message
			
			//Test if you want to break
			strncpy(breakStatement, buf, strlen(breakStatement)); // Copy the first few indices of buf to 'breakStatement'
			
			
			// If so, break
			if ((breakStatement[0] == escapeChar[0]) && (breakStatement[1] == escapeChar[1]) && (breakStatement[2] == escapeChar[2])) {// If buffer[0, 1, 2] is escape sequence
				printf("%s, you have left the chat.",name2);
				printf("\n");
				exit(0); // Break the loop and call it a day
			}
			
			memset(buf, 0, sizeof(buf)); // Clear the buffer
			
			memset(breakStatement, 0, sizeof(breakStatement)); // Clear 'breakStatement' for next time
			
     
      // Communicate until break triggered
      while(1)
      {
				memset(buf, 0, sizeof(buf)); // Clear the buffer
				
				memset(breakStatement, 0, sizeof(breakStatement)); // Clear 'breakStatement' for next time
				
				n = recv(sd, buf, sizeof(buf), 0); // Receive the name
				
				printf("%d",n);
				
				//memset(buf, 0, sizeof(buf)); // Clear the buffer
				int i;
				for (i=0; i<sizeof(breakStatement); i++) {
					breakStatement[i] = buf[i];
				}
				
        // If so, break
       if ((breakStatement[0] == escapeChar[0]) && (breakStatement[1] == escapeChar[1]) && (breakStatement[2] == escapeChar[2])) {// If buffer[0, 1, 2] is escape sequence
          printf("%s Has Left The Chat.",name);
					printf("\n");
					exit(0); // Break the loop and call it a day
        }
       
        // Else, continue
        else {
					
          memset(breakStatement, 0, sizeof(breakStatement)); // Clear 'breakStatement' for next time
        
          // Prints client's stuff to screen
          printf("%s says: \n", name); // Print the name
					
          write(1, buf, n); // Print the text of the message
         
					printf("\n"); // Make it pretty	
					
					memset(buf, 0, sizeof(buf)); // Clear the buffer
					
          // Say something back
          printf("Say something back (or type '%s' to quit): ", escapeChar); // Prompt
				
				  gets(buf); // Accepts your text
					
          // Prints your stuff to screen
          printf("%s says:\n ", name2); // Prints your name
					
          write(1, buf, strlen(buf)); // Print the text of your message
					
					printf("\n"); // Make it pretty	
         
          // Send it back and get ready for the next message
          send(sd, buf, strlen(buf), 0); // Send your message
					
					for (i=0; i<sizeof(breakStatement); i++) {
						breakStatement[i] = buf[i];
					}
					
					// If so, break
					if ((breakStatement[0] == escapeChar[0]) && (breakStatement[1] == escapeChar[1]) && (breakStatement[2] == escapeChar[2])) {// If buffer[0, 1, 2] is escape sequence
						printf("%s Has Left The Chat.",name2);
						printf("\n");
						exit(0); // Break the loop and call it a day
					}
					
          memset(buf, 0, sizeof(buf)); // Clear the buffer
					memset(breakStatement, 0, sizeof(breakStatement)); // Clear 'breakStatement' for next time
					
        }
       
      }
     
    }
   
	// tell server to stop
	buf[0]=0xff;
	send(sd,buf,1,0);
	
	printf("Client is stopping\n");
	closesocket(sd);
 
	exit(0);
}
