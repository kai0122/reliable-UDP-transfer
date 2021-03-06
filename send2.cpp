// Timeout: alarm
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <netdb.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sstream>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>



using namespace std;

#define MAXDATA   20480
#define MAXSEQ  100
#define MAXNAME 1000
#define NDG      2000   /* datagrams to send */
#define DGLEN   65507   /* length of each datagram */
#define TIMEOUT 1

void sig_alrm(int signo){
    cout << "ALARM...\n";
    alarm(TIMEOUT);
    return;
}

int main(int argc, char **argv){
	int SERV_PORT = atoi(argv[2]);
    int sockFd;
    if ((sockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        perror("socket() error");

    struct sockaddr_in srvAddr;
    bzero(&srvAddr, sizeof(srvAddr));
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(SERV_PORT);
    if (inet_pton(AF_INET, argv[1], &srvAddr.sin_addr) <= 0)
        perror("inet_pton() error");

    if (connect(sockFd, (const struct sockaddr *) &srvAddr, sizeof(srvAddr)) < 0)
        perror("connect() error");

    

    //  *****************************************************
    //                      Read From File
    //  *****************************************************

    long lSize;
    size_t result;
    FILE *readFile, *writeFile;

    // Open file for both reading and writing
    readFile = fopen(argv[3], "r+");
    writeFile = fopen("oldBig", "w+");
    

    // obtain file size:
    fseek (readFile , 0 , SEEK_END);
    lSize = ftell (readFile);
    rewind (readFile);
    

    
    vector<char *> vec;
    
    size_t bytesRead = 0;
    size_t prev;
    char *buffer;
    buffer = (char*) malloc (MAXDATA);
    if (readFile != NULL)    
    {
        // read up to sizeof(buffer) bytes
        
        while ((bytesRead = fread(buffer, 1, MAXDATA, readFile)) > 0)
        {
            // process bytesRead worth of data in buffer
            vec.push_back(buffer);
            prev = bytesRead;
            buffer = (char*) malloc (MAXDATA);
            //fwrite(vec[vec.size()-1], 1, bytesRead, writeFile);
        }
    }
    for(int i=0;i<vec.size();i++){
        if(i != vec.size()-1){
            fwrite(vec[i], 1, MAXDATA, writeFile); 
        }
        else{
            fwrite(vec[i], 1, prev, writeFile);
        }

    }
    fclose(readFile);
    fclose(writeFile);
    

    //  *****************************************************
    //                 Send total to Server
    //  *****************************************************

    
    int s = -2;
    char seq[MAXSEQ];
    sprintf(seq, "%d", s);
    char NUM[MAXDATA] = {};
    sprintf(NUM, "%d", (int)vec.size());
    char sendMsg1[MAXDATA+MAXSEQ] = {};
    char dataGet1[MAXDATA+MAXSEQ] = {};
        
    for(int in = 0;in < MAXDATA+MAXSEQ;in++){
        if(in<MAXSEQ)
            sendMsg1[in] = seq[in];
        else
            sendMsg1[in] = NUM[in-MAXSEQ];
    }

    
   
    while(1){
        send(sockFd, sendMsg1, MAXDATA+MAXSEQ + 1, 0);
        
        
        signal(SIGALRM, sig_alrm);
        siginterrupt(SIGALRM, 1);
        alarm(TIMEOUT);
        int recv_size = recv(sockFd, dataGet1, MAXDATA+MAXSEQ, 0);
        if (recv_size == -1)
        {   
            if(errno == EINTR)
                cout << "TOTAL TIME OUT...\n";    
            else
                cout << "recvfrom error...\n";
            continue;
        }
        alarm(0);
        
        int j;
        for(j=0;j<MAXDATA+MAXSEQ;j++){
            if(sendMsg1[j] != dataGet1[j])
                break;
        }
        if(j == MAXDATA+MAXSEQ){
            cout << "TOTAL SAME\n";
            break;
        }
        else{
            //cout << sendMsg1[j] << ", " << dataGet1[j] << endl;
        }
    }
    cout << string(NUM) << endl;
    
    //  *****************************************************
    //                 Send prev to Server
    //  *****************************************************
    
    s = -1;
    char seq2[MAXSEQ];
    sprintf(seq2, "%d", s);
    char PREV[MAXDATA] = {};
    sprintf(PREV, "%d", (int)prev);
    char sendMsg2[MAXDATA+MAXSEQ] = {};
    char dataGet2[MAXDATA+MAXSEQ] = {};
        
    for(int in = 0;in < MAXDATA+MAXSEQ;in++){
        if(in<MAXSEQ)
            sendMsg2[in] = seq2[in];
        else
            sendMsg2[in] = PREV[in-MAXSEQ];
    }
    
    
    while(1){
        send(sockFd, sendMsg2, MAXDATA+MAXSEQ + 1, 0);


        signal(SIGALRM, sig_alrm);
        siginterrupt(SIGALRM, 1);
        alarm(TIMEOUT);
        int recv_size = recv(sockFd, dataGet2, MAXDATA+MAXSEQ, 0);
        if (recv_size == -1)
        {
            if(errno == EINTR)
                cout << "PREV TIME OUT...\n";    
            else
                cout << "recvfrom error...\n";
            continue;
        }
        alarm(0);
            
        int j;
        for(j=0;j<MAXDATA+MAXSEQ;j++){
            if(sendMsg2[j] != dataGet2[j])
                break;
        }
        if(j == MAXDATA+MAXSEQ){
        	cout << "PREV SAME\n";
            break;
        }
    }
    cout << string(PREV) << endl;

    //  *****************************************************
    //                 Send data to Server
    //  *****************************************************
    signal(SIGALRM, sig_alrm);
    for(int i=0;i<=vec.size()+1;i++){
        
        //  *****************************
        //      Add sequence number
        //  *****************************

        char seq[MAXSEQ];
        sprintf(seq, "%d", i);
        char sendMsg[MAXDATA+MAXSEQ] = {};
        char dataGet[MAXDATA+MAXSEQ] = {};

        if(i == vec.size()){
            // Send File Name
            char name[MAXDATA];
            for(int t=0;t<strlen(argv[3]);t++)
                name[t] = argv[3][t];

            for(int in = 0;in < MAXDATA+MAXSEQ;in++){
                if(in<MAXSEQ)
                    sendMsg[in] = seq[in];
                else if((in-MAXSEQ) < strlen(argv[3]))
                    sendMsg[in] = name[in-MAXSEQ];
                else
                    sendMsg[in] = '\0';
            }
        }
        else if(i == vec.size()+1){
            // Send END
            for(int in = 0;in < MAXDATA+MAXSEQ;in++){
                if(in<MAXSEQ)
                    sendMsg[in] = seq[in];
                else
                    sendMsg[in] = 'X';
            }
        }
        else{
            for(int in = 0;in < MAXDATA+MAXSEQ;in++){
                if(in<MAXSEQ)
                    sendMsg[in] = seq[in];
                else
                    sendMsg[in] = vec[i][in-MAXSEQ];
            }   
        }

        
        while(1){
            //cout << "len1: " << strlen(sendMsg) << endl;
            int NUM_TO_SEND;
            if(i == vec.size()-1)
                NUM_TO_SEND = prev+MAXSEQ;
            else
                NUM_TO_SEND = MAXDATA+MAXSEQ;
            if (send(sockFd, sendMsg, NUM_TO_SEND, 0) < 0){
                perror("write to server error !");
                exit(1);
            }


            signal(SIGALRM, sig_alrm);
            siginterrupt(SIGALRM, 1);
            alarm(TIMEOUT);
            int recv_size;
            if ((recv_size = recv(sockFd, dataGet, NUM_TO_SEND, 0)) < 0)
            {
                if(errno == EINTR)
                    cout << "TIME OUT...\n";    
                else
                    cout << "recvfrom error...\n";
                continue;
            }
            alarm(0);
            
            int j;
            for(j=0;j<NUM_TO_SEND;j++){
                if(sendMsg[j] != dataGet[j])
                    break;
            }
            if(j == NUM_TO_SEND){
                cout << "ACK: " << i << endl;
                break;
            }
            else{
                cout << "ACK error... " << i << ", " << j  << ", " << NUM_TO_SEND << endl;

                char recvSeq[MAXSEQ];
                for(int k=0;k<MAXSEQ;k++)
                    recvSeq[k] = dataGet[k];
                cout << "GET: " << string(recvSeq) << endl;
                    //do {
                    //    cout << '\n' << "Press the Enter key to continue.";
                    //   } while (cin.get() != '\n');

                    
            }

  
        }


    }
    cout << "Send Done...\n";

    
    
    
    


    for(int i=0;i<vec.size();i++)
        free(vec[i]);

    /*

    for(int i=0;i<vec.size();i++){
        //cout << "send: " << i << endl;
        char sendMsg[MAXDATA] = {};
        char dataGet[MAXDATA] = {};
        for(int in = 0;in < MAXDATA;in++)
            sendMsg[in] = vec[i][in];

        
        
        while(1){
            //cout << "len1: " << strlen(sendMsg) << endl;
            int NUM_TO_SEND;
            if(i == vec.size()-1)
                NUM_TO_SEND = prev;
            else
                NUM_TO_SEND = MAXDATA;
            if (send(sockFd, sendMsg, NUM_TO_SEND, 0) < 0){
                perror("write to server error !");
                exit(1);
            }

            
            if (recv(sockFd, dataGet, NUM_TO_SEND, 0) < 0) {
                perror ("read from server error !");
                exit (1);
            }
            
            int j;
            for(j=0;j<NUM_TO_SEND;j++){
                if(sendMsg[j] != dataGet[j])
                    break;
            }
            if(j == NUM_TO_SEND){
                cout << "ACK: " << i << endl;
                break;
            }
            else{
                //cout << "asd" << j << endl;
                cout << "ACK error... " << i << endl;
            }
            
        }


    } 
    cout << "Send Done...\n";

    
    fclose(ones);  
    
    


    for(int i=0;i<vec.size();i++)
        free(vec[i]);
     
        
    */
    

}