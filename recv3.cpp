// Timeout: select
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#define MAXNAME 1000
#define MAXDATA   20480
#define MAXSEQ  100

extern int errno;

int main(int argc, char **argv)
{
	int SERV_PORT = atoi(argv[1]);
	int fileDes;    /* file description */
	int sockfd;
	int recfd;
	int length;
	int nRead;
	struct sockaddr_in myAddr;
	struct sockaddr_in clientAddr;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	bzero((char *)&myAddr, sizeof(myAddr));
	myAddr.sin_family = AF_INET;
	myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myAddr.sin_port = htons(SERV_PORT);

	if (bind(sockfd, (struct sockaddr *)&myAddr, sizeof(myAddr)) <0) {
    	perror ("bind failed\n");
        exit(1);
    }

	length = sizeof(clientAddr);
	cout << "Server is ready to receive !!\n";
    cout << "Can strike Cntrl-c to stop Server >>\n";

    while(1){
    	vector<char *> vec;
    	//	*************************************************
    	//				Get First number
    	//	*************************************************

    	int total;
	    int prev;
	    char recvSeq[MAXSEQ];
	    char *data = (char*) malloc (MAXDATA);   	
    	while(1){
    		char NUM[MAXDATA+MAXSEQ] = {};
    		if ((nRead = recvfrom(sockfd, &NUM, MAXDATA+MAXSEQ, 0, (struct sockaddr*)&clientAddr, (socklen_t *)&length)) <0) {
		        perror ("could not read datagram!!");
		        continue;
		    }

		    char one[MAXSEQ];
	        sprintf(one, "%d", -2);
	        char two[MAXSEQ];
	        sprintf(two, "%d", -1);
	        char three[MAXSEQ];
	        sprintf(three, "%d", 0);
	        	
	        
	        	
	        	
	        for(int k=0;k<MAXSEQ;k++)
	        	recvSeq[k] = NUM[k];
	        for(int k=MAXSEQ;k<MAXSEQ+MAXDATA;k++)
	        	data[k-MAXSEQ] = NUM[k];

	        if(strcmp(recvSeq, one) == 0){
	        	cout << "GET TOTAL\n";
		    	string str1(data);
		    	istringstream is1(data);
		   	 	is1 >> total;
		   	 	
	        	if (sendto(sockfd, NUM, MAXDATA+MAXSEQ + 1, 0, (struct sockaddr*)&clientAddr, length) < 0) {
		            perror("Could not send datagram!!\n");
		            continue;
		        }
	        }
	        else if(strcmp(recvSeq, two) == 0){
	        	cout << "GET PREV\n";
	        	string str2(data);
		    	istringstream is2(str2);
		   	 	is2 >> prev;

	        	if (sendto(sockfd, NUM, MAXDATA+MAXSEQ + 1, 0, (struct sockaddr*)&clientAddr, length) < 0) {
		            perror("Could not send datagram!!\n");
		            continue;
		        }
	        }
	        else if(strcmp(recvSeq, three) == 0){
	        	// Get the first sequence -> TOTAL and PREV were both Correct
	        	cout << "Correct: " << string(recvSeq) << endl;
	        	if(vec.size() == 1){
	        		// get before
	        		strcpy(vec[vec.size()-1], data);
	        	}
	        	else{
		        	vec.push_back(data);	
	        		data = (char*) malloc (MAXDATA);
	        	}

				if (sendto(sockfd, NUM, MAXDATA+MAXSEQ + 1, 0, (struct sockaddr*)&clientAddr, length) < 0) {
		            perror("Could not send datagram!!\n");
		            continue;
		        }       	
	        	break;
	        }
	        else{
	        	if (sendto(sockfd, NUM, MAXDATA+MAXSEQ + 1, 0, (struct sockaddr*)&clientAddr, length) < 0) {
		            perror("Could not send datagram!!\n");
		            continue;
		        }
	        }
    	}
		cout << "TOTAL: " << total << endl;
		cout << "PREV: " << prev << endl;
	    

   	 	//	*************************************************
    	//						Get Data
    	//	*************************************************

	    string fileName;
	    int count = 1;

	    char *buffer;
	    char *data2;
	    data2 = (char*) malloc (MAXDATA);
    	buffer = (char*) malloc (MAXDATA+MAXSEQ);
    	char *prevBuffer;
    	while(count <= total+1){
    		//cout << count << endl;
    		
    		if ((nRead = recvfrom(sockfd, buffer, MAXDATA+MAXSEQ, 0, (struct sockaddr*)&clientAddr, (socklen_t *)&length)) <0) {
	            perror ("could not read datagram!!");
	            continue;
	        }
	        
	        //	*****************************
	        //		Get Sequence number
	        //	*****************************

	        char seq[MAXSEQ];
        	sprintf(seq, "%d", count);
        	
        	char recvSeq[MAXSEQ];
        	
        	
        	for(int k=0;k<MAXSEQ;k++)
        		recvSeq[k] = buffer[k];
        	for(int k=MAXSEQ;k<MAXSEQ+MAXDATA;k++)
        		data2[k-MAXSEQ] = buffer[k];

        	if(count == total){
        		fileName = string(data2);
        		cout << "File name: " << string(data2) << endl;
        		sendto(sockfd, buffer, MAXDATA+MAXSEQ + 1, 0, (struct sockaddr*)&clientAddr, length);
        		count++;
        		continue;
        	}
        	else if(count == total + 1){
        		if(strcmp(recvSeq, seq) != 0){
					fileName = string(data2);
        			cout << "File name: " << string(data2) << endl;
        			sendto(sockfd, buffer, MAXDATA+MAXSEQ + 1, 0, (struct sockaddr*)&clientAddr, length);
        			continue;
        		}
        		else{
        			cout << "RECV END...\n";
        			count++;
        			break;
        		}
        	}

        	if(strcmp(recvSeq, seq) == 0){
        		cout << "Correct: " << string(seq) << endl;
        		cout << "count: " << count << endl;
        		vec.push_back(data2);

        		if (sendto(sockfd, buffer, MAXDATA+MAXSEQ + 1, 0, (struct sockaddr*)&clientAddr, length) < 0) {
		            perror("Could not send datagram!!\n");
		            continue;
		        }
		        data2 = (char*) malloc (MAXDATA);
		        count++;
        	}
        	else{
        		cout << "NACK: " << string(recvSeq) << ", " << string(seq) << endl;
        		strcpy(vec[vec.size()-1], data2);

        		if (sendto(sockfd, buffer, MAXDATA+MAXSEQ + 1, 0, (struct sockaddr*)&clientAddr, length) < 0) {
		            perror("Could not send datagram!!\n");
		            continue;
		        }
        	}

    	}

    	

    	printf("Received data form %s : %d\n", inet_ntoa(clientAddr.sin_addr), htons(clientAddr.sin_port));
        //printf("%s\n", buffer);
    	
    	
    	FILE *wFile;
        wFile = fopen(fileName.c_str(), "w+");
        for(int i=0;i<vec.size();i++){
	        if(i != vec.size()-1){
	            fwrite(vec[i], 1, MAXDATA, wFile); 
	        }
	        else{
	        	fwrite(vec[i], 1, prev, wFile); 
	        }
        
    	}
    	fclose(wFile);

    	cout << "Write File done...\n";
    	for(int i=0;i<vec.size();i++)
        	free(vec[i]);


        printf("Can Strike Crtl-c to stop Server >>\n");

    }

    
	return 0;
}