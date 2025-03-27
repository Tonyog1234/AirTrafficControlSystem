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
    pthread_detach(displayThread);
}

void* Display::DisplayServerThread(void* arg) {
    Display* self = static_cast<Display*>(arg);
    while (true) {
        int rcvid;
        msg_struct msg;
        struct _msg_info info;
        rcvid = MsgReceive(self->attach->chid, &msg, sizeof(msg), &info);
        if (rcvid == -1) {
            perror("[Display] MsgReceive failed");
            continue;
        }

        time_t now = time(NULL);
        char time_str[26];
        ctime_r(&now, time_str);
        time_str[strlen(time_str) - 1] = '\0';  // Remove newline

        // Log sender PID and message
        cout << "[" << time_str << "] [Display] Received - Sender PID: " << info.pid
             << ", ID: " << msg.id << ", Body: \"" << msg.body << "\"" << endl;

        // Filter valid messages
        if (strncmp(msg.body, "ID:", 3) == 0 || strncmp(msg.body, "Aircraft ID", 11) == 0) {
            cout << "[" << time_str << "] [Display] Aircraft Info - " << msg.body << endl;
        } else {
            cout << "[" << time_str << "] [Display] Ignoring invalid message - Sender PID: " << info.pid
                 << ", ID: " << msg.id << ", Body: \"" << msg.body << "\"" << endl;
            MsgReply(rcvid, 0, NULL, 0);
            continue;
        }

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
        pause();
    }
    return 0;
}
