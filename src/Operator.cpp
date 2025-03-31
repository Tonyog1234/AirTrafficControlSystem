#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/dispatch.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <cstdlib>  // For system("clear")
using namespace std;

typedef struct {
    unsigned int id;
    char body[500];
} msg_struct;

void* CommandAircraft(void* arg) {
    name_attach_t *attach;
    attach = name_attach(NULL, "myserver", 0);
    if (attach == NULL) {
        perror("name_attach");
        pthread_exit(NULL);
    }

    cout << "Command Server is running, waiting for alert messages...\n";

    while (true) {
        int rcvid;
        msg_struct msg;
        rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);
        if (rcvid == -1) {
            perror("MsgReceive failed");
            continue;
        }

        cout << "Received alert from ComputerSystem: " << msg.body << endl;
        cout << "Enter new speed command for Aircraft " << msg.id << " (e.g., 'speedX speedY speedZ'): ";

        string command;
        getline(cin, command);

        msg_struct reply;
        reply.id = msg.id;
        strncpy(reply.body, command.c_str(), sizeof(reply.body) - 1);
        reply.body[sizeof(reply.body) - 1] = '\0';

        if (MsgReply(rcvid, 0, &reply, sizeof(reply)) == -1) {
            perror("MsgReply failed");
        } else {
            cout << "Sent speed command: " << reply.body << endl;
        }
    }

    name_detach(attach, 0);
    return NULL;
}

void* RequestInfo(void* arg) {
	 int coid = name_open("RequestInfo", 0);
	    if (coid == -1) {
	        perror("name_open");

	    }
	    int UserInput;
	    cout<<"[Operator Console] Enter Aircraft ID for Display: ";
	    cin>>UserInput;

	    msg_struct msgToComputer; // Message structure
	    msgToComputer.id = UserInput;
	    strcpy(msgToComputer.body, "Hello from Operator");
	    std::cout << "[Operator Console] Sending message to server: " << msgToComputer.body << std::endl;

	    msg_struct replyFromComputer;

	    // Send message to the server and get a reply
	    int status = MsgSend(coid, &msgToComputer, sizeof(msgToComputer), &replyFromComputer, sizeof(replyFromComputer));
	    if (status == -1) {
	        perror("MsgSend");

	    }

	    // Display the server's reply
	    std::cout << "[Operator Console] Received reply from server: " << replyFromComputer.body << std::endl;

	    name_close(coid); // Close connection to the server

    return NULL;
}

int main() {
    pthread_t infoThread;
    // Start RequestInfo interactive thread
    if (pthread_create(&infoThread, NULL, RequestInfo, NULL) != 0) {
        cerr << "Error creating RequestInfo thread" << endl;
        return 1;
    }

    pthread_join(infoThread, NULL);
    return 0;
}
