#include "ComputerSystem.h"
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cmath>
#include <cstring>
#include <errno.h>
#include <time.h>
using namespace std;

ComputerSystem::ComputerSystem() {
    OpenSharedMemory();
    InitializeSemaphore();
    ReadData();
    CheckForAlerts();
    StartInfoServer();
}

ComputerSystem::~ComputerSystem() {
    if (timer_id != 0) {
        timer_delete(timer_id);
        timer_id = 0;
    }
    if (attach != nullptr) {
        name_detach(attach, 0);
    }
    CloseSharedMemory();
    DestroySemaphore();
}

bool ComputerSystem::getAlert() {
    return alert;
}

void ComputerSystem::OpenSharedMemory() {
    const char* SHM_NAME = "/aircraft_shm";
    shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("[ComputerSystem] shm_open failed");
        exit(EXIT_FAILURE);
    }

    shm_ptr = mmap(nullptr, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("[ComputerSystem] mmap failed");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }
}

void ComputerSystem::CloseSharedMemory() {
    if (shm_ptr && shm_ptr != MAP_FAILED) {
        munmap(shm_ptr, SHM_SIZE);
        shm_ptr = nullptr;
    }
    if (shm_fd != -1) {
        close(shm_fd);
        shm_fd = -1;
    }
}

sem_t* ComputerSystem::shm_sem = nullptr;

void ComputerSystem::InitializeSemaphore() {
    const char* SEM_NAME = "/aircraft_sem";
    shm_sem = sem_open(SEM_NAME, 0);
    if (shm_sem == SEM_FAILED) {
        perror("[ComputerSystem] sem_open failed");
        exit(EXIT_FAILURE);
    }
}

void ComputerSystem::DestroySemaphore() {
    if (shm_sem != SEM_FAILED && shm_sem != nullptr) {
        sem_close(shm_sem);
        shm_sem = nullptr;
    }
}

void ComputerSystem::ReadData() {
    sem_wait(shm_sem);
    aircraftList.clear();
    int count = *(static_cast<int*>(shm_ptr));
    if (count < 0 || count > MAX_AIRCRAFT) {
        cerr << "[ComputerSystem] Invalid aircraft count: " << count << endl;
        sem_post(shm_sem);
        return;
    }
    AircraftData* dataArray = reinterpret_cast<AircraftData*>(
        static_cast<char*>(shm_ptr) + sizeof(int)
    );
    for (int i = 0; i < count; i++) {
        aircraftList.push_back(dataArray[i]);
    }
    sem_post(shm_sem);
}

void ComputerSystem::CheckForAlerts() {
    alert = false;
    for (const auto& ac : aircraftList) {
        bool outOfBounds =
            (ac.x < MIN_X || ac.x > MAX_X ||
             ac.y < MIN_Y || ac.y > MAX_Y ||
             ac.z < MIN_Z || ac.z > MAX_Z);
        if (outOfBounds) {
            cerr << "[ALERT] Aircraft " << ac.id << " is out of bounds!" << endl;
            alert = true;
        }
    }
    for (size_t i = 0; i < aircraftList.size(); i++) {
        for (size_t j = i + 1; j < aircraftList.size(); j++) {
            double dx = aircraftList[i].x - aircraftList[j].x;
            double dy = aircraftList[i].y - aircraftList[j].y;
            double dz = aircraftList[i].z - aircraftList[j].z;
            double horizontalDist = sqrt(dx * dx + dy * dy);
            double verticalDist = fabs(dz);
            if (horizontalDist < MIN_HORIZONTAL_SEPARATION &&
                verticalDist < MIN_VERTICAL_SEPARATION) {
                cerr << "[ALERT] Aircraft " << aircraftList[i].id
                     << " and Aircraft " << aircraftList[j].id
                     << " are too close! (Horizontal Dist=" << horizontalDist
                     << ", Vertical Dist=" << verticalDist << ")" << endl;
                alert = true;
            }
        }
    }
}

void ComputerSystem::print() {
    for (const auto& ad : aircraftList) {
        cout << "Flight ID: " << ad.id << endl;
        cout << "Flight Position: (" << ad.x << ", " << ad.y << ", " << ad.z << ")" << endl;
        cout << "Flight Speed: (" << ad.speedX << ", " << ad.speedY << ", " << ad.speedZ << ")" << endl;
        cout << "Flight Status: " << ad.status << endl;
        cout << "****************************" << endl;
    }
}

void ComputerSystem::RequestCommand() {
    int coid = name_open("myserver", 0);
    if (coid == -1) {
        perror("[ComputerSystem] name_open failed");
        return;
    }

    msg_struct msg;
    msg.id = aircraftList[0].id;
    strcpy(msg.body, "Alert detected, request speed change");

    cout << "[ComputerSystem] Sending alert to operator: " << msg.body << endl;

    msg_struct reply;
    int status = MsgSend(coid, &msg, sizeof(msg), &reply, sizeof(reply));
    if (status == -1) {
        perror("[ComputerSystem] MsgSend failed");
        name_close(coid);
        return;
    }

    cout << "[ComputerSystem] Received speed command: " << reply.body << endl;
    cout << "[ComputerSystem] Speed command '" << reply.body << "' received but not implemented." << endl;

    name_close(coid);
}

void* ComputerSystem::InfoServerThread(void* arg) {
	ComputerSystem* self = static_cast<ComputerSystem*>(arg);
	    cout << "[COMPUTER SYSTEM] Server is running, waiting for messages...\n";

	    while (true) {
	        int rcvid;
	        msg_struct msgFromOperator;
	        rcvid = MsgReceive(self->attach->chid, &msgFromOperator, sizeof(msgFromOperator), NULL);

	        if (rcvid == -1) {
	            perror("No Msg Received");
	            continue;
	        }
	        if (rcvid <= 0) {
	            continue;
	        }
	        if (msgFromOperator.id == 0 || strlen(msgFromOperator.body) == 0) {
	            continue;
	        }

	        cout << "[COMPUTER SYSTEM] Received message: " << msgFromOperator.body  << msgFromOperator.id << std::endl;

	        //Send msg to Display

	        self->ReadData();
	        string replyBody;
	               bool found = false;
	               for (const auto& ad : self->aircraftList) {
	                   if (ad.id == static_cast<int>(msgFromOperator.id)) {
	                       found = true;
	                       replyBody = "ID: " + to_string(ad.id) +
	                                   ", Pos: (" + to_string(static_cast<int>(ad.x)) + "," +
	                                                to_string(static_cast<int>(ad.y)) + "," +
	                                                to_string(static_cast<int>(ad.z)) + ")" +
	                                   ", Speed: (" + to_string(static_cast<int>(ad.speedX)) + "," +
	                                                  to_string(static_cast<int>(ad.speedY)) + "," +
	                                                  to_string(static_cast<int>(ad.speedZ)) + ")"

													  + ", Status: " + (ad.status ? "Active" : "Inactive");
	                       break;
	                   }
	               }
	               if (!found)
	            	   replyBody="Aircraft not found";

	        msg_struct msgToDisplay;
	        msgToDisplay.id = msgFromOperator.id;

	        strncpy(msgToDisplay.body, replyBody.c_str(), sizeof(msgToDisplay.body) - 1);
	        msgToDisplay.body[sizeof(msgToDisplay.body) - 1] = '\0';

	        cout << "[COMPUTER SYSTEM] Send message: " << msgToDisplay.body << std::endl;


	        //Send info to Display
	        msg_struct replyFromDisplay;
	        int coid_comp = name_open("DisplayInfo", 0);
	        	    if (coid_comp == -1) {
	        	        perror("name_open");

	        	    }
	        int status = MsgSend(coid_comp, &msgToDisplay, sizeof(msgToDisplay), &replyFromDisplay, sizeof(replyFromDisplay));
	       	    if (status == -1) {
	       	        perror("MsgSend");

	       	    }

	       	    //Reply to operator
	        msg_struct replyToOperator;
	        replyToOperator.id = msgFromOperator.id;
	        strcpy(replyToOperator.body, "Message Received!");
	        MsgReply(rcvid, 0, &replyToOperator, sizeof(replyToOperator));
	    }

	    name_detach(self->attach, 0);
    return NULL;
}

void ComputerSystem::StartInfoServer() {
    attach = name_attach(NULL, "RequestInfo", 0);
    if (attach == NULL) {
        perror("[ComputerSystem] name_attach failed");
        exit(EXIT_FAILURE);
    }

    pthread_t infoThread;
    if (pthread_create(&infoThread, NULL, InfoServerThread, this) != 0) {
        cerr << "Error creating InfoServer thread" << endl;
        exit(EXIT_FAILURE);
    }
    pthread_detach(infoThread);
}

void ComputerSystem::TimerHandler(union sigval sv) {
    auto* self = static_cast<ComputerSystem*>(sv.sival_ptr);
   // self->ReadData();
   // self->CheckForAlerts();
    if (self->getAlert()) {
        // self->RequestCommand();
    }
}

void ComputerSystem::StartTimer() {
    struct sigevent sev;
    struct itimerspec its;

    memset(&sev, 0, sizeof(sev));
    memset(&its, 0, sizeof(its));

    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = TimerHandler;
    sev.sigev_value.sival_ptr = this;

    if (timer_create(CLOCK_REALTIME, &sev, &timer_id) == -1) {
        cerr << "[ComputerSystem] Error creating timer: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }

    its.it_value.tv_sec = 2;
    its.it_interval.tv_sec = 5;

    if (timer_settime(timer_id, 0, &its, nullptr) == -1) {
        cerr << "[ComputerSystem] Error setting timer: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }
}

void* ComputerThread(void* arg) {
    ComputerSystem* ptr = static_cast<ComputerSystem*>(arg);
    ptr->StartTimer();
    while (true) {
        pause();
    }
    return nullptr;
}

int main() {
    ComputerSystem cs;
    pthread_t thread;

    if (pthread_create(&thread, NULL, ComputerThread, &cs) != 0) {
        cerr << "Error creating thread" << endl;
        return 1;
    }

    pthread_join(thread, NULL);
    return 0;
}
