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
    while (true) {
        system("clear");  // Clear terminal before prompt
        cout << "Enter aircraft ID to request condition (or 'q' to quit): ";
        string input;
        getline(cin, input);

        if (input == "q" || input == "Q") {
            break;
        }

        int aircraftId;
        try {
            aircraftId = stoi(input);
        } catch (...) {
            cout << "Invalid ID. Please enter a number.\n";
            sleep(1);  // Pause to show error
            continue;
        }

        int coid = name_open("computer_system", 0);
        if (coid == -1) {
            perror("name_open failed");
            continue;
        }

        msg_struct msg;
        msg.id = aircraftId;
        strcpy(msg.body, "Request aircraft condition");

        cout << "Sending request for Aircraft " << msg.id << endl;

        msg_struct reply;
        int status = MsgSend(coid, &msg, sizeof(msg), &reply, sizeof(reply));
        if (status == -1) {
            perror("MsgSend failed");
            name_close(coid);
            continue;
        }

        cout << "Received condition: " << reply.body << endl;
        name_close(coid);
    }
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
