
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SIZE 13
#define MAX_NAME 10
char buf[SIZE];
char buf0[MAX_NAME];
char buf1[16];
char buf2[SIZE];
char buf3[SIZE];
char buf4[SIZE];

/**
 * auxiliary function to do the treatment of client2:
 * connect,send nickname,receive server's greeting,send the message MAX, receive server's answer,disconnect
 * @param sockt
 * @return
 */
int max_int_protocol(int sockt) {
    printf("Hello Client2 , Please enter your nickname:\n");
    int rd1 = read(STDIN_FILENO,buf0,MAX_NAME+1);
    if(rd1<0){
        perror("Error reading client2's message");
        return 1;
    }

    // sending nickname
    if ((send(sockt, buf0, MAX_NAME, 0)) == -1) {
        perror("Error sending client2's nickname ");
        return 1;
    }

    //getting server's greeting
    int recu1 = recv(sockt, buf1, 16 * sizeof(char), 0);
    if (recu1 == -1) {
        perror("Error receiving server's greeting message");
        return 1;
    }
    buf1[recu1] = '\0';
    printf("\nServer's answer to client2:\n%s\n", buf1);


    printf("Client2, please enter your message to the server\n");
    int rd = read(STDIN_FILENO,buf,MAX_NAME+1);
    if(rd<0){
        perror("Error reading client's message");
        return 1;
    }

   // sending the message MAX
    if ((send(sockt, buf, MAX_NAME, 0)) == -1) {
        perror("Error sending client2's answer");
        return 1;
    }
    // le client2 receives server's reply
    int recu2 = recv(sockt, buf2, 14 * sizeof(char), 0);
    if (recu2 == -1) {
        perror("Error receiving server's answer");
        return 1;
    }
    buf2[recu2] = '\0';
    //parsing the reply
    strncpy(buf3,buf2,3);

    if (strcmp(buf3, "REP") == 0) {
        //parsing the server's answer to get the nickname
        strncpy(buf4,buf2+3,10);

        uint32_t adress;
        uint16_t val;
        char* str=malloc(sizeof(char)*15);
        if(str==NULL){
            perror("error malloc");
            return 1;
        }
        //getting the adress & converting it to host presentation
        if(read(sockt, &adress,sizeof(adress))==-1) {
            perror("error receiving adress");
            return 1;
        }

        //getting the max value & converting it to little endian
        if( read(sockt,&val, sizeof(val))==-1){
            perror("error receiving max");
            return 1;
        }
        printf("\nServer's answer : \n%s%s%u\n",buf2,inet_ntop(AF_INET,&adress,str,INET_ADDRSTRLEN),ntohs(val));
        printf("\nClient's nickname : %s\n",buf4);
        printf("the ip adress is : %s\n",inet_ntop(AF_INET,&adress,str,INET_ADDRSTRLEN));
        printf("the maximum is : %u\n",ntohs(val));

        free(str);
    } else if (strcmp(buf3, "NOP") == 0) {
        printf("Server's answer : %s\n",buf2);

        printf("Server didn't receive the number yet\n You may try later?\n");

    }

    return 0;
}
/***********************************************************************************************************************/
//                                                    The main
/***********************************************************************************************************************/
int main(int argc, char * argv[]){

    if (argc!=3){
        fprintf(stderr,"You entered less arguments than expected!\n"
                       "Please retry with the correct number of arguments\n");
        exit(EXIT_FAILURE);
    }
    char * adress= argv[1];
    int port= atoi(argv[2]);

    //init the structure
    struct sockaddr_in adresse_socket;
    adresse_socket.sin_family=AF_INET;
    adresse_socket.sin_port=htons(port);
    int n= inet_aton(adress,&adresse_socket.sin_addr);
    if (n==0){
        perror("Error creating socket adress");
        exit(EXIT_FAILURE);
    }

    //creating socket
    int sockt= socket(PF_INET,SOCK_STREAM,0);
    if(sockt==-1){
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    //connecting
    int conct= connect(sockt,(struct sockaddr *)&adresse_socket,sizeof( struct sockaddr_in));
    if(conct==-1){
        perror("Error connect");
        exit(EXIT_FAILURE);
    }
    //do the treatment
    if ((max_int_protocol( sockt))==1){
        exit(EXIT_FAILURE);
    }

    // close connection
    close(sockt);
    return 0;
}