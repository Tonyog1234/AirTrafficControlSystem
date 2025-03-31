#include "Display.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <time.h>
using namespace std;

Display::Display() {
    StartDisplayServer();
}

Display::~Display() {
    if (attach != nullptr) {
        name_detach(attach, 0);
    }
}

void Display::StartDisplayServer() {
    cout << "Display server start........" << endl;
    attach = name_attach(NULL, "DisplayInfo", 0);
    if (attach == NULL) {
        perror("[Display] name_attach failed");
        exit(EXIT_FAILURE);
    }

    pthread_t displayThread;
    if (pthread_create(&displayThread, NULL, DisplayServerThread, this) != 0) {
        cerr << "[Display] Error creating DisplayServer thread" << endl;
        name_detach(attach, 0);
        exit(EXIT_FAILURE);
    }
    pthread_detach(displayThread);
}

void* Display::DisplayServerThread(void* arg) {
    Display* self = static_cast<Display*>(arg);
    while (true) {
        int rcvid;
        msg_struct msgFromComp;
        //struct _msg_info info;
        rcvid = MsgReceive(self->attach->chid, &msgFromComp, sizeof(msgFromComp), NULL);
        if (rcvid == -1) {
            perror("[Display] MsgReceive failed");
            continue;
        }

       cout<<"[Display] Receive message from computer "<<endl;
       cout<<"[Display] Aircraft Info: "<<msgFromComp.body<<endl;
        msg_struct replyToComp;
        replyToComp.id = msgFromComp.id;
        strcpy(replyToComp.body, "Displayed Successfully");
        if (MsgReply(rcvid, 0, &replyToComp, sizeof(replyToComp)) == -1) {
            perror("[Display] MsgReply failed");
        }
    }
    return NULL;
}

int main() {
    Display display;
    while (true) {
        pause();
    }
    return 0;
}
