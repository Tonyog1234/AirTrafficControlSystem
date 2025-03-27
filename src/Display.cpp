#include "Display.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
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
	cout<<"Display server start........"<<endl;
    attach = name_attach(NULL, "display_system", 0);
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
    pthread_detach(displayThread);  // Detach so it runs independently
}

void* Display::DisplayServerThread(void* arg) {
    Display* self = static_cast<Display*>(arg);
    while (true) {
        int rcvid;
        msg_struct msg;
        rcvid = MsgReceive(self->attach->chid, &msg, sizeof(msg), NULL);
        if (rcvid == -1) {
            perror("[Display] MsgReceive failed");
            continue;
        }

        cout << "[Display] Aircraft Info - " << msg.body << endl;

        // Reply with an acknowledgment
        msg_struct reply;
        reply.id = msg.id;
        strcpy(reply.body, "Displayed");
        if (MsgReply(rcvid, 0, &reply, sizeof(reply)) == -1) {
            perror("[Display] MsgReply failed");
        }
    }
    return NULL;
}

int main() {
    Display display;
    while (true) {
        pause();  // Keep main thread alive
    }
    return 0;
}
