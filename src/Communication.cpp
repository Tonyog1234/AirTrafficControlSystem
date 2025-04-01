#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cmath>
#include <cstring>
#include <errno.h>
#include <time.h>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/dispatch.h>
using namespace std;

typedef struct {
    unsigned int id;
    char body[500];
} msg_struct;

void CommandToAircraft(msg_struct& msgFromComp){

	int coid_air;
	        while (true) {
	        	coid_air = name_open("Air", 0);
	            if (coid_air == -1) {
	                cerr << "[Communication] Waiting for CommandAircraft server to start..." << endl;
	                sleep(1); // Wait and retry
	                continue;
	            }
	            break; // Connected successfully
	        }
	    msg_struct msgToAir;
	    msgToAir.id = msgFromComp.id;
	    strncpy(msgToAir.body, msgFromComp.body, sizeof(msgToAir.body) - 1);
	    msgToAir.body[sizeof(msgToAir.body) - 1] = '\0';

	    cout<<"[Communication] Send Speed Modification to Aircraft: "<<msgToAir.body<<endl;

	    msg_struct replyFromAir;
	    int status_comm = MsgSend(coid_air, &msgToAir, sizeof(msgToAir), &replyFromAir, sizeof(replyFromAir));
	    if (status_comm == -1) {
	       perror("[Communication] MsgSend failed");
	       name_close(coid_air);
	       return;
	    }
	    cout << "[Communication] Received Reply from Aircraft: " << replyFromAir.body << endl;
	    name_close(coid_air);

}
void StartServer(){
	name_attach_t* attach = name_attach(NULL, "Communication", 0);
	cout<<"Communication server start......."<<endl;
    if (attach == NULL) {
        perror("[ComputerSystem] name_attach failed");
        exit(EXIT_FAILURE);
    }
    while (true) {
           int rcvid;
           msg_struct msgFromComp;
           rcvid = MsgReceive(attach->chid, &msgFromComp, sizeof(msgFromComp), NULL);

           if (rcvid == -1) {
               perror("No Msg Received");
               continue;
           }
           if (rcvid <= 0) {
               continue;
           }
           if (msgFromComp.id == 0 || strlen(msgFromComp.body) == 0) {
               continue;
           }

           cout << "[Communication] Received message: " << msgFromComp.body << endl;
           msg_struct replyToComp;
           replyToComp.id = msgFromComp.id;
           strcpy(replyToComp.body, "Message Received from Communication");
           MsgReply(rcvid, 0, &replyToComp, sizeof(replyToComp));
           CommandToAircraft(msgFromComp);
    }
    name_detach(attach, 0);

}

int main() {
	StartServer();
	return 0;
}
