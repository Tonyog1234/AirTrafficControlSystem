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
        attach = nullptr; // Ensure we don’t double-detach
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
    // Do NOT detach here; let the thread run and detach in the destructor
    pthread_detach(displayThread);
}

void* Display::DisplayServerThread(void* arg) {
    Display* self = static_cast<Display*>(arg);
    cout << "[Display] Server is running, waiting for messages...\n";

    while (true) {
        int rcvid;
        msg_struct msgFromComp;

        rcvid = MsgReceive(self->attach->chid, &msgFromComp, sizeof(msgFromComp), NULL);
        if (rcvid == -1) {
            // Only print error if it’s a real issue; otherwise, just loop
            if (errno != EAGAIN && errno != EINTR) { // Ignore transient errors
                perror("[Display] MsgReceive failed");
            }
            usleep(100000); // Sleep briefly to avoid tight loop
            continue;
        }

        if (rcvid > 0) { // Valid message received
            cout << "[Display] Receive message from computer" << endl;
            cout << "[Display] Aircraft Info: " << msgFromComp.body << endl;

            msg_struct replyToComp;
            replyToComp.id = msgFromComp.id;
            strcpy(replyToComp.body, "Displayed Successfully");

            if (MsgReply(rcvid, 0, &replyToComp, sizeof(replyToComp)) == -1) {
                perror("[Display] MsgReply failed");
            }
        }
    }
    return NULL;
}

int main() {
    Display display;
    while (true) {
        pause(); // Keep main thread alive
    }
    return 0;
}
