#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/dispatch.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <cstdlib>
#include <fstream>
#include <ctime>
using namespace std;

typedef struct {
    unsigned int id;
    char body[500];
} msg_struct;

static bool alertOutofBound = false;
static bool alertCollision = false;

static pthread_mutex_t alertOutofBoundMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t alertCollisionMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t ioMutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for I/O
static pthread_mutex_t logMutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for file logging

// Function to get current timestamp as a string
string getTimestamp() {
    time_t now = time(NULL);
    string timestamp = ctime(&now);
    // Remove trailing newline from ctime
    if (!timestamp.empty() && timestamp[timestamp.length() - 1] == '\n') {
        timestamp.pop_back();
    }
    return timestamp;
}

// Function to log to file
void logToFile(const string& message) {
    pthread_mutex_lock(&logMutex);
    ofstream logFile("operator_log.txt", ios::app); // Open in append mode
    if (logFile.is_open()) {
        logFile << "[" << getTimestamp() << "] " << message << endl;
        logFile.close();
    } else {
        cerr << "Failed to open log file!" << endl;
    }
    pthread_mutex_unlock(&logMutex);
}

void* CollisionAircraft(void* arg) {
    name_attach_t *attach;
    attach = name_attach(NULL, "CollisionAircraft", 0);
    if (attach == NULL) {
        perror("name_attach for CollisionAircraft");
        pthread_exit(NULL);
    }

    pthread_mutex_lock(&ioMutex);
    cout << "CollisionAircraft Server is running, waiting for collision alert messages...\n";
    pthread_mutex_unlock(&ioMutex);

    while (true) {
        int rcvid;
        msg_struct msgFromComp;
        rcvid = MsgReceive(attach->chid, &msgFromComp, sizeof(msgFromComp), NULL);
        if (rcvid == -1) {
            perror("MsgReceive failed in CollisionAircraft");
            continue;
        }
        if (rcvid <= 0 || msgFromComp.id == 0 || strlen(msgFromComp.body) == 0) {
            continue;
        }

        pthread_mutex_lock(&alertCollisionMutex);
        alertCollision = true;
        pthread_mutex_unlock(&alertCollisionMutex);

        pthread_mutex_lock(&ioMutex);
       // cout << "Received collision alert from ComputerSystem: " << msgFromComp.body << endl;
        cout << "[URGENT COLLISION] Enter new speed command for Aircraft " << msgFromComp.id << endl;
        string command;
        cout << "Enter new speeds (e.g., 100 200 300): ";
        pthread_mutex_unlock(&ioMutex);

        getline(cin, command);

        pthread_mutex_lock(&ioMutex);
       // cout << "Captured collision command: '" << command << "'\n";
        pthread_mutex_unlock(&ioMutex);

        // Log the collision command
        string logMessage = "Collision Command for Aircraft " + to_string(msgFromComp.id) + ": " + command;
        logToFile(logMessage);

        msg_struct replyToComp;
        replyToComp.id = msgFromComp.id;
        memset(replyToComp.body, 0, sizeof(replyToComp.body));
        strncpy(replyToComp.body, command.c_str(), sizeof(replyToComp.body) - 1);
        replyToComp.body[sizeof(replyToComp.body) - 1] = '\0';

        if (MsgReply(rcvid, 0, &replyToComp, sizeof(replyToComp)) == -1) {
            perror("MsgReply failed in CollisionAircraft");
        } else {
            pthread_mutex_lock(&ioMutex);
            cout << "Sent collision speed command: " << replyToComp.body << endl;
            pthread_mutex_unlock(&ioMutex);
        }

        pthread_mutex_lock(&alertCollisionMutex);
        alertCollision = false;
        pthread_mutex_unlock(&alertCollisionMutex);
    }

    name_detach(attach, 0);
    return NULL;
}

void* CommandAircraft(void* arg) {
    name_attach_t *attach;
    attach = name_attach(NULL, "CommandAircraft", 0);
    if (attach == NULL) {
        perror("name_attach for CommandAircraft");
        pthread_exit(NULL);
    }

    pthread_mutex_lock(&ioMutex);
    cout << "CommandAircraft Server is running, waiting for out-of-bounds alert messages...\n";
    pthread_mutex_unlock(&ioMutex);

    while (true) {
        int rcvid;
        msg_struct msgFromComp;
        rcvid = MsgReceive(attach->chid, &msgFromComp, sizeof(msgFromComp), NULL);
        if (rcvid == -1) {
            perror("MsgReceive failed in CommandAircraft");
            continue;
        }
        if (rcvid <= 0 || msgFromComp.id == 0 || strlen(msgFromComp.body) == 0) {
            continue;
        }

        pthread_mutex_lock(&alertOutofBoundMutex);
        alertOutofBound = true;
        pthread_mutex_unlock(&alertOutofBoundMutex);

        pthread_mutex_lock(&ioMutex);
        //cout << "Received out-of-bounds alert from ComputerSystem: " << msgFromComp.body << endl;
        cout << "[URGENT OUT OF BOUNDS] Enter new speed command for Aircraft " << msgFromComp.id << endl;
        string command;
        cout << "Enter new speeds (e.g., 100 200 300): ";
        pthread_mutex_unlock(&ioMutex);

        getline(cin, command);

        pthread_mutex_lock(&ioMutex);
       // cout << "Captured out-of-bounds command: '" << command << "'\n";
        pthread_mutex_unlock(&ioMutex);

        // Log the out-of-bounds command
        string logMessage = "Out-of-Bounds Command for Aircraft " + to_string(msgFromComp.id) + ": " + command;
        logToFile(logMessage);

        msg_struct replyToComp;
        replyToComp.id = msgFromComp.id;
        memset(replyToComp.body, 0, sizeof(replyToComp.body));
        strncpy(replyToComp.body, command.c_str(), sizeof(replyToComp.body) - 1);
        replyToComp.body[sizeof(replyToComp.body) - 1] = '\0';

        if (MsgReply(rcvid, 0, &replyToComp, sizeof(replyToComp)) == -1) {
            perror("MsgReply failed in CommandAircraft");
        } else {
            pthread_mutex_lock(&ioMutex);
            cout << "Sent out-of-bounds speed command: " << replyToComp.body << endl;
            pthread_mutex_unlock(&ioMutex);
        }

        pthread_mutex_lock(&alertOutofBoundMutex);
        alertOutofBound = false;
        pthread_mutex_unlock(&alertOutofBoundMutex);
    }

    name_detach(attach, 0);
    return NULL;
}

void RequestInfoForDisplay(int coid_op) {
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

    // Log the info request
    string logMessage = "Info Request for Aircraft " + to_string(msgToComputer.id);
    logToFile(logMessage);

    pthread_mutex_lock(&ioMutex);
   // cout << "[Operator Console] Sending message to server: " << msgToComputer.body << endl;
    pthread_mutex_unlock(&ioMutex);

    msg_struct replyFromComputer;
    int status = MsgSend(coid_op, &msgToComputer, sizeof(msgToComputer), &replyFromComputer, sizeof(replyFromComputer));
    if (status == -1) {
        perror("MsgSend");
    } else {
        pthread_mutex_lock(&ioMutex);
       // cout << "[Operator Console] Received reply from Computer System: " << replyFromComputer.body << endl;
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
        pthread_mutex_lock(&alertCollisionMutex);
        bool isCollisionActive = alertCollision;
        pthread_mutex_unlock(&alertCollisionMutex);

        pthread_mutex_lock(&alertOutofBoundMutex);
        bool isOutofBoundActive = alertOutofBound;
        pthread_mutex_unlock(&alertOutofBoundMutex);

        if (isCollisionActive) {
            usleep(100000); // 100ms delay
        } else if (isOutofBoundActive) {
            usleep(100000); // 100ms delay
        } else {
            RequestInfoForDisplay(coid_op);
            usleep(1000);
        }
    }

    name_close(coid_op);
    return NULL;
}

int main() {
    pthread_t commandThread, collisionThread, consoleThread;

    if (pthread_create(&collisionThread, NULL, CollisionAircraft, NULL) != 0) {
        cerr << "Error creating CollisionAircraft thread" << endl;
        return 1;
    }
    if (pthread_create(&commandThread, NULL, CommandAircraft, NULL) != 0) {
        cerr << "Error creating CommandAircraft thread" << endl;
        return 1;
    }
    if (pthread_create(&consoleThread, NULL, OperatorConsole, NULL) != 0) {
        cerr << "Error creating OperatorConsole thread" << endl;
        return 1;
    }

    pthread_join(collisionThread, NULL);
    pthread_join(commandThread, NULL);
    pthread_join(consoleThread, NULL);
    return 0;
}
