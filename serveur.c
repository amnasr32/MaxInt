
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

pthread_mutex_t verrou= PTHREAD_MUTEX_INITIALIZER;

// data structure for storing the client's connection (ip adress & socket)
typedef struct  {
    struct sockaddr_in* adresse_ip;
    int  socket;
    char* add;
}info;

// data structure that contains data of the client who send the biggest integer
typedef struct  {

    uint16_t max;
    char* pseudo ;
    char* add;
}maxin;

// declaring the global variable
maxin max_info={.max=0,.pseudo=NULL,.add=NULL};

#define BUF_SIZE 50
#define MAX_NAME 10

void* maxint (void* args );

//buffer for HELLO nickname
char buff2[BUF_SIZE];
// buffer for receiving the instruction
char buff3[BUF_SIZE];
// buffer for
char buff4[BUF_SIZE];
// buffer for
char buff5[BUF_SIZE];
//buffer for sending the
char buff6[BUF_SIZE];
//buffer for sending the server's answer to client2
char buff7[BUF_SIZE];

/***********************************************************************************************************************/

/**
 * This functions aims to receive client's nickname (length=10), then send the server's respond "HELLO nickname"
 * @param sock
 * @return client's nickname
 */
char* greeting(info* data){

    // receiving the client's nickname
    char* buff1=malloc(sizeof(char)*MAX_NAME);
    int recu1 = recv(data->socket, buff1, (MAX_NAME ) * sizeof(char), 0);
    if (recu1 == -1) {
        perror("Error receiving client's nickname");
        exit(EXIT_FAILURE);
    }
    /**
     * Practically this is not the best way to do it but in case a client send a nickname
     * with length<10 a blank space will be added until nickname length become=10.
     */
    if(strlen(buff1)<MAX_NAME){
        //
        for(int i=strlen(buff1);i<MAX_NAME;i++){
            buff1[i] = ' ';
        }
    }
    buff1[recu1] = '\0';

    sprintf(buff2, "HELLO %s", buff1);
    if ((send(data->socket, buff2, strlen(buff2), 0)) == -1) {
        perror("Error greeting client ");
        exit(EXIT_FAILURE);
    }

    printf("Server's answer to client: %s\n", buff2);
    //free(buff1); // tried to free here but caused damage to my code ('sorry couldn't fix it')
    return buff1;
}
/***********************************************************************************************************************/
/**
 * function that treats the received instruction from client1 "INT"
 * @param sock
 * @param n
 * @param nickname
 * @param data
 * @return
 */
int INT_instruction(info* data,int n,char* nickname){
    pthread_mutex_lock(&verrou);

    //updating the structure's member's
    if(n>=max_info.max){
        max_info.max=n;
        max_info.pseudo=nickname;
        max_info.add=data->add;
    }
    pthread_mutex_unlock(&verrou);
    //sending the server's reply
    sprintf(buff6, "INTOK");
    if ((send(data->socket, buff6, strlen(buff6), 0)) == -1) {
        perror("Error answering client");
        return 1;
    }
    printf("Server's answer to client: %s\n", buff6);
    return 0;
}
/***********************************************************************************************************************/

/**
 * function that treats the received instruction from client2 "MAX"
 * @param sock
 * @return
 */
int MAX_instruction(info* data){
    if ((max_info.max)==0){
        sprintf(buff7,"NOP");
        if ((send(data->socket, buff7, strlen(buff7), 0)) == -1) {
            perror("Error sending NOP");
            return 1;
        }
    }else {

        //sending the REPnickname
        sprintf(buff7,"REP%s\n",max_info.pseudo);
        if ((send(data->socket, buff7, strlen(buff7), 0)) == -1) {
            perror("Error sending REPpseudo");
            return 1;
        }
        uint32_t adress;
        //converting the maximum to network presentation (big endian)
        uint16_t  max =(htons(max_info.max));
        //converting adress to network presentation
        if (inet_pton(AF_INET,max_info.add,&adress)==-1){
            perror("Converting adresse to network in server");
            return 1;
        }
        // sending the adress
        if ((send(data->socket, &adress, sizeof(adress),0)) == -1) {
            perror("Error sending ip");
            return 1;
        }
        //sending the maximum
        if ((send(data->socket, &max, sizeof(max),0)) == -1) {
            perror("Error sending max");
            return 1;
        }

    }
    return 0;
}







/***************************the auxiliary function (argument of function pthread_create)*********************************/

void* maxint (void* args ) {

    info* data=((info *)args);

    /****************** Greeting client *********************/
    char* nickname= greeting(data);
   /****************** treatment pour les deux clients *******************/

    // the server receives the instruction
    int recu3 = recv(data->socket, buff3, (BUF_SIZE - 1) * sizeof(char), 0);
    if (recu3 == -1) {
        perror("Error receiving client1's answer");
        exit(EXIT_FAILURE);
    }
    buff3[recu3] = '\0';
    strncpy(buff4, buff3, 3);
    printf("server received instruction : %s\n",buff4);

    if ((strcmp(buff4, "INT") == 0)) {

        strcpy(buff5, buff3 + 4);
        if(atoi(buff5)<0){
            perror("you have entered a negative number!\n"
                   "you are now banned! Good bye!");
            exit(EXIT_FAILURE);
        }else {
            uint16_t n = ntohs((uint16_t)atoi(buff5));
            printf("client's chosen number : %u\n", n);
            if((INT_instruction(data,n,nickname))==1){
                perror("Error answering client for instruction INT");
                exit(EXIT_FAILURE);
            }
        }

    }
    else if ((strcmp(buff4, "MAX") == 0)) {
        if((MAX_instruction(data))==1){
            exit(EXIT_FAILURE);
        }
    }
    /****************** End of treatment for clients *******************/
    if ((close(data->socket)) == -1) {
        perror("Error closing client's socket");
        exit(EXIT_FAILURE);
    }
    printf("******End of protocol maxint******\n");
    printf("\n\n");
    return NULL;
    }




/***********************************************************************************************************************/
//                                                    The main
/***********************************************************************************************************************/

int main (int argc , char** argv){

    if (argc!=2){
        fprintf(stderr,"You entered less arguments than expected!\n"
               "Please retry with the correct number of arguments\n");
        exit(EXIT_FAILURE);
    }
    // on convertit le port passé en paramètre en entier
    int port=atoi(argv[1]);
    int sock_server= socket(PF_INET,SOCK_STREAM,0);
    if(sock_server==-1){
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in adress_sock;
    adress_sock.sin_family=AF_INET;
    adress_sock.sin_port=htons(port);
    adress_sock.sin_addr.s_addr=htonl(INADDR_ANY);


    // associate the socket to a port
    int r1= bind(sock_server,(struct sockaddr *)&adress_sock, sizeof(struct sockaddr_in));
    if(r1==-1){
        perror("Error bind");
        exit(EXIT_FAILURE);
    }

    int l = listen(sock_server, 0);
    if(l==-1){
        perror("Error listen");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in *caller=malloc(sizeof (struct sockaddr_in));
    socklen_t size = sizeof(caller);
    printf("***********************************************************\n"
           "*           Welcome to 'Protocol of max-integer'          *\n"
           "***********************************************************\n\n");
    while (1) {


        int* client =(int *) malloc(sizeof (int));
        if (client==NULL){
            perror("Error memory allocation");
            exit(EXIT_FAILURE);
        }
        //the server accepts client's connection
         *client  = accept(sock_server, (struct sockaddr *) caller, &size);
        /*** Init the structure ***/
        info *param =malloc(sizeof( info));
        param->add=malloc(sizeof(char*));
        if(param==NULL){
            perror("erreur allocation pour la structure");
            exit(EXIT_FAILURE);
        }
        param->adresse_ip=caller;
        param->socket=*client;
        strcpy(param->add, inet_ntoa((param->adresse_ip)->sin_addr));

        printf("\n********************Start communication***************\n");
        printf("   client is connected with success \n");
        printf("   on socket: %d\n",param->socket);
        printf("   client's adress :%s\n",param->add);

        if ((param->socket >= 0) ) {

            pthread_t th1;
            pthread_create(&th1,NULL,maxint,param);
            free(client);
            //free(param);  // tried to free here but caused damage to my code ('sorry couldn't fix it')
            //free(param->add);
        }

    }
    free(caller);
    close(sock_server);
    return 0;
}
