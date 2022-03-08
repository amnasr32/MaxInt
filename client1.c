
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>




#define SIZE 20
#define MAX_NAME 10

char buf0[MAX_NAME];
char buf1[SIZE];
char buf2[SIZE];
char buf3[SIZE];
char buf4[SIZE];
char buf5[SIZE];

/**
 * auxiliary function to do the treatment of client1:
 * connect,send nickname,receive server's greeting,send the integer,disconnect =>(x5)
 * @param adresse_socket
 * @return NULL
 */
void* max_int_protocol(struct sockaddr_in adresse_socket){



    for(int i=0;i<5;i++){
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

        /************************************************************/
        printf("Hello Client1 , Please enter your nickname:\n");

        if((read(STDIN_FILENO,buf0,MAX_NAME+1))==-1){
            perror("error reading client's nickname");
            exit(EXIT_FAILURE);
        }
        //sending nickname to server
        if ((send(sockt, buf0, MAX_NAME, 0))==-1){
                perror("Error sending client1's nickname ");
                exit(EXIT_FAILURE);
        }
        // receiving server's greeting
        int recu2 = recv(sockt, buf1,16* sizeof(char), 0) ;
        if(recu2==-1){
            perror("Error receiving server's greeting message");
            exit(EXIT_FAILURE);
        }
        buf1[recu2] = '\0';

        printf("\n---------You may choose a number---------\n");
        int rd = read(STDIN_FILENO,buf2,SIZE);;
        if(rd<0){
           perror("Error reading client's number");
           exit(EXIT_FAILURE);
        }
        //parsing client's message to get the number chosen
        strcpy(buf3,buf2+4);
        //client choose a negative number he will be banned and connection will be closed
        if(atoi(buf3)<0) {
            perror("you have entered a negative number!\n"
                   "you are now banned! Good bye!");
            close(sockt);
            exit(EXIT_FAILURE);
        }else {
            //converting the number to little endian
            int16_t k = htons((int16_t)atoi(buf3));

            //generating the answer
            sprintf(buf4,"INT %u",k);
            //sending the number
            if ((send(sockt, buf4, strlen(buf4), 0))==-1){
                perror("Error sending client1's chosen number ");
                exit(EXIT_FAILURE);
            }
            int recu3 = recv(sockt, buf5, (SIZE-1) * sizeof(char), 0) ;
            if(recu3==-1){
                perror("Error receiving server's answer");
                exit(EXIT_FAILURE);
            }
            buf5[recu3]='\0';
            printf("\nServer's answer to client1:\n%s\n", buf5);
            printf("\n*******Client finished treatment*******\n");
            close(sockt);
        }

    }


    return NULL;

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
    int port= atoi(argv[2]);
    char * adress= argv[1];

    //init the structure
    struct sockaddr_in adresse_socket;
    adresse_socket.sin_family=AF_INET;
    adresse_socket.sin_port=htons(port);
    int n= inet_aton(adress,&adresse_socket.sin_addr);
    if (n==0){
        perror("Error generating socket adress");
        exit(EXIT_FAILURE);
    }

    //do the treatment
    max_int_protocol(adresse_socket);

    return 0;


}