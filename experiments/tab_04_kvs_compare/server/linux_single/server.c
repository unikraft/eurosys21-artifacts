#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <assert.h>
#include <sys/types.h> 
#include <sys/socket.h> 
//#include <arpa/inet.h> 
#include <netinet/in.h> 
  
#define PORT     9001
#define MAXLINE 1300
  
// Driver code 
int main() { 
    int sockfd; 
    char buffer[MAXLINE]; 
    struct sockaddr_in servaddr, cliaddr; 
      
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sockfd > 2);
    
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 
      
    assert(bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) == 0); 
      
    int len, ret1, ret2; 
  
    len = sizeof(cliaddr);
    printf("Server started \n");
    while (1) { 
    	ret1 = recvfrom(sockfd, (char *)buffer, MAXLINE,  
                MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
                &len);


    	ret2 = sendto(sockfd, (const char *)buffer, ret1,  
        	0, (const struct sockaddr *) &cliaddr, 
            	len);

	assert(ret1 >= 0);
	assert(ret2 >= 0);
    }
      
    return 0; 
} 


