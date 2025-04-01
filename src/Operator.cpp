#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/dispatch.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <cstdlib>
using namespace std;

typedef struct {
    unsigned int id;
    char body[500];
} msg_struct;

static bool alertActive = false;
static pthread_mutex_t alertMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t ioMutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for I/O

void* CommandAircraft(void* arg) {
    name_attach_t *attach;
    attach = name_attach(NULL, "CommandAircraft", 0);
    if (attach == NULL) {
        perror("name_attach");
        pthread_exit(NULL);
    }

    pthread_mutex_lock(&ioMutex);
    cout << "CommandAircraft Server is running, waiting for alert messages...\n";
    pthread_mutex_unlock(&ioMutex);

    while (true) {
        int rcvid;
        msg_struct msgFromComp;
        rcvid = MsgReceive(attach->chid, &msgFromComp, sizeof(msgFromComp), NULL);
        if (rcvid == -1) {
            perror("MsgReceive failed");
            continue;
        }
        if (rcvid <= 0 || msgFromComp.id == 0 || strlen(msgFromComp.body) == 0) {
            continue;
        }

        pthread_mutex_lock(&alertMutex);
        alertActive = true;
        pthread_mutex_unlock(&alertMutex);

        pthread_mutex_lock(&ioMutex);
        cout << "Received alert from ComputerSystem: " << msgFromComp.body << endl;
        cout << "[URGENT] Enter new speed command for Aircraft " << msgFromComp.id << endl;
        string Vx, Vy, Vz;
        cout << "Enter speed for x: ";
        cin >> Vx;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear buffer
        cout << "Enter speed for y: ";
        cin >> Vy;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear buffer
        cout << "Enter speed for z: ";
        cin >> Vz;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear buffer
        string command = Vx + " " + Vy + " " + Vz;

        cout << "Captured command: '" << command << "'\n";
        pthread_mutex_unlock(&ioMutex);

        msg_struct replyToComp;
        replyToComp.id = msgFromComp.id;
        memset(replyToComp.body, 0, sizeof(replyToComp.body));
        strncpy(replyToComp.body, command.c_str(), sizeof(replyToComp.body) - 1);
        replyToComp.body[sizeof(replyToComp.body) - 1] = '\0';

        if (MsgReply(rcvid, 0, &replyToComp, sizeof(replyToComp)) == -1) {
            perror("MsgReply failed");
        } else {
            pthread_mutex_lock(&ioMutex);
            cout << "Sent speed command: " << replyToComp.body << endl;
            pthread_mutex_unlock(&ioMutex);
        }

        pthread_mutex_lock(&alertMutex);
        alertActive = false;
        pthread_mutex_unlock(&alertMutex);
    }

    name_detach(attach, 0);
    return NULL;
}

void RequestInfo(int coid_op) {
    pthread_mutex_lock(&ioMutex);
    int userInput;
    cout << "[Operator Console] Enter Aircraft ID for Display (or 0 to wait): ";
    cin >> userInput;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    pthread_mutex_unlock(&ioMutex);

    if (userInput == 0) {
        return;
    }

    msg_struct msgToComputer;
    msgToComputer.id = userInput;
    string replybody = "Request Info of Aircraft " + to_string(msgToComputer.id);
    strncpy(msgToComputer.body, replybody.c_str(), sizeof(msgToComputer.body) - 1);
    msgToComputer.body[sizeof(msgToComputer.body) - 1] = '\0';

    pthread_mutex_lock(&ioMutex);
    cout << "[Operator Console] Sending message to server: " << msgToComputer.body << endl;
    pthread_mutex_unlock(&ioMutex);

    msg_struct replyFromComputer;
    int status = MsgSend(coid_op, &msgToComputer, sizeof(msgToComputer), &replyFromComputer, sizeof(replyFromComputer));
    if (status == -1) {
        perror("MsgSend");
    } else {
        pthread_mutex_lock(&ioMutex);
        cout << "[Operator Console] Received reply from Computer System: " << replyFromComputer.body << endl;
        pthread_mutex_unlock(&ioMutex);
    }
}

void* OperatorConsole(void* arg) {
    int coid_op;
    while (true) {
        coid_op = name_open("RequestInfo", 0);
        if (coid_op == -1) {
            pthread_mutex_lock(&ioMutex);
            cerr << "Waiting for RequestInfo server to start..." << endl;
            pthread_mutex_unlock(&ioMutex);
            sleep(1);
            continue;
        }
        break;
    }

    while (true) {
        pthread_mutex_lock(&alertMutex);
        bool isAlert = alertActive;
        pthread_mutex_unlock(&alertMutex);

        if (!isAlert) {
            RequestInfo(coid_op);
            sleep(1); //Need to check if it's needed
        } else {
            usleep(100000); // 100ms delay during alert
        }
    }

    name_close(coid_op);
    return NULL;
}

int main() {
    pthread_t commandThread, consoleThread;

    if (pthread_create(&commandThread, NULL, CommandAircraft, NULL) != 0) {
        cerr << "Error creating CommandAircraft thread" << endl;
        return 1;
    }

    if (pthread_create(&consoleThread, NULL, OperatorConsole, NULL) != 0) {
        cerr << "Error creating OperatorConsole thread" << endl;
        return 1;
    }

    pthread_join(commandThread, NULL);
    pthread_join(consoleThread, NULL);
    return 0;
}
