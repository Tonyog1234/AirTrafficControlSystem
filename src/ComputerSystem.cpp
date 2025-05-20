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

bool ComputerSystem::getAlertOutofBound() {
    return AlertOutofBound;
}

bool ComputerSystem::getAlertCollision(){
	return AlertCollision;
}
int ComputerSystem::getOneTimeBound() {
    return onetimeBound;
}
int ComputerSystem::getOneTimeCollision(){
	return onetimeCollision;
}
int ComputerSystem::getCollisionCount(){
	return CollisionCount;
}
void ComputerSystem::setCollisionCount(){
	CollisionCount=0;
}
void ComputerSystem::IncrementCollisionCount(){
	CollisionCount++;
}
void ComputerSystem::setOneTimeCollision(){
	onetimeCollision=1;
}
int ComputerSystem::getBoundCount(){
	return BoundCount;
}
void ComputerSystem::setBoundCount(){
	BoundCount=0;
}
void ComputerSystem::IncrementBoundCount(){
	BoundCount++;
}
void ComputerSystem::setOneTimebound(){
	onetimeBound=1;
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
        cerr << "[COMPUTER SYSTEM] Invalid aircraft count: " << count << endl;
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

void ComputerSystem::OutofBoundAlerts() {
    AlertOutofBound = false;

    for (const auto& ac : aircraftList) {
        bool outOfBounds =
            (ac.x < MIN_X || ac.x > MAX_X ||
             ac.y < MIN_Y || ac.y > MAX_Y ||
             ac.z < MIN_Z || ac.z > MAX_Z);
        if (outOfBounds) {
            cout << "[ALERT OUT OF BOUND] Aircraft " << ac.id << " is out of bounds!" << endl;
            AlertOutofBound = true;
            onetimeBound--;
            index = ac.id;
            alertedAircraft.insert(ac.id);
        }
    }
}

void ComputerSystem::CollisionAlerts(){
	AlertCollision=false;

	for (size_t i = 0; i < aircraftList.size(); i++) {
	        for (size_t j = i + 1; j < aircraftList.size(); j++) {
	            double dx = aircraftList[i].x - aircraftList[j].x;
	            double dy = aircraftList[i].y - aircraftList[j].y;
	            double dz = aircraftList[i].z - aircraftList[j].z;
	            double horizontalDist = sqrt(dx * dx + dy * dy);
	            double verticalDist = fabs(dz);
	            if (horizontalDist < MIN_HORIZONTAL_SEPARATION &&
	                verticalDist < MIN_VERTICAL_SEPARATION) {
	                cerr << "[ALERT COLLISION] Aircraft " << aircraftList[i].id
	                     << " and Aircraft " << aircraftList[j].id
	                     << " are too close! (Horizontal Dist=" << horizontalDist
	                     << ", Vertical Dist=" << verticalDist << ")" << endl;
	                AlertCollision = true;
	                CollisionIndex=i;// Assume smaller ID has to change its speed
	                onetimeCollision--;
	            }
	        }
	}
}
void ComputerSystem::SolveCollision(){
	int coid_op;
	    while (true) {
	        coid_op = name_open("CollisionAircraft", 0);
	        if (coid_op == -1) {
	            cerr << "[COMPUTER SYSTEM] Waiting for CollisionAircraft server to start..." << endl;
	            sleep(1); // Wait and retry
	            continue;
	        }
	        break; // Connected successfully
	    }
	    //Operator
	        msg_struct msgToOperator;
	        msgToOperator.id = aircraftList[CollisionIndex].id; // Note: Check if index-1 is correct
	        strcpy(msgToOperator.body, "Collision detected, request speed change");

	       // cout << "[COMPUTER SYSTEM] Sending Collision Alert to Operator: " << msgToOperator.body << endl;

	        msg_struct replyFromOperator;
	        int status = MsgSend(coid_op, &msgToOperator, sizeof(msgToOperator), &replyFromOperator, sizeof(replyFromOperator));
	        if (status == -1) {
	            perror("[COMPUTER SYSTEM] MsgSend failed");
	            name_close(coid_op);
	            return;
	        }
	        //cout << "[COMPUTER SYSTEM] Received Speed Command from Operator: " << replyFromOperator.body << endl;

	        name_close(coid_op);

	  //Communication
	  int coid_comm;
	   while (true) {
	       coid_comm = name_open("Comm_Collision", 0);
	       if (coid_comm == -1) {
	          cerr << "[COMPUTER SYSTEM] Waiting for Communication server to start..." << endl;
	          sleep(1); // Wait and retry
	          continue;
	        }
	         break; // Connected successfully
	   }
	           msg_struct msgToComm;
	           msgToComm.id = replyFromOperator.id;
	           strncpy(msgToComm.body, replyFromOperator.body, sizeof(msgToComm.body) - 1);
	           msgToComm.body[sizeof(replyFromOperator.body) - 1] = '\0';

	          //cout<<"[COMPUTER SYSTEM] Send Speed Modification to Communication: "<<msgToComm.body<<endl;
	           msg_struct replyFromComm;
	           int status_comm = MsgSend(coid_comm, &msgToComm, sizeof(msgToComm), &replyFromComm, sizeof(replyFromComm));
	           if (status_comm == -1) {
	              perror("[COMPUTER SYSTEM] MsgSend failed");
	              name_close(coid_comm);
	              return;
	           }
	          // cout << "[COMPUTER SYSTEM] Received Reply from Communication: " << replyFromComm.body << endl;
	           onetimeCollision = 1;
	           onetimeBound=1;
	           setOneTimeCollision();
	           name_close(coid_comm);
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
	IncrementBoundCount();
    int coid_op;
    while (true) {
        coid_op = name_open("CommandAircraft", 0);
        if (coid_op == -1) {
            cerr << "[COMPUTER SYSTEM] Waiting for CommandAircraft server to start..." << endl;
            sleep(1); // Wait and retry
            continue;
        }
        break; // Connected successfully
    }
    //Operator
    msg_struct msgToOperator;
    msgToOperator.id = aircraftList[index - 1].id; // Note: Check if index-1 is correct
    strcpy(msgToOperator.body, "Alert detected, request speed change");

   // cout << "[COMPUTER SYSTEM] Sending Out of Bound Alert to Operator: " << msgToOperator.body << endl;

    msg_struct replyFromOperator;
    int status = MsgSend(coid_op, &msgToOperator, sizeof(msgToOperator), &replyFromOperator, sizeof(replyFromOperator));
    if (status == -1) {
        perror("[COMPUTER SYSTEM] MsgSend failed");
        name_close(coid_op);
        return;
    }

    //cout << "[COMPUTER SYSTEM] Received speed command: " << replyFromOperator.body << endl;

    name_close(coid_op);

    //Communication
    int coid_comm;
        while (true) {
        	coid_comm = name_open("Communication", 0);
            if (coid_comm == -1) {
                cerr << "[COMPUTER SYSTEM] Waiting for Communication server to start..." << endl;
                sleep(1); // Wait and retry
                continue;
            }
            break; // Connected successfully
        }
    msg_struct msgToComm;
    msgToComm.id = replyFromOperator.id;
    strncpy(msgToComm.body, replyFromOperator.body, sizeof(msgToComm.body) - 1);
    msgToComm.body[sizeof(replyFromOperator.body) - 1] = '\0';

    cout<<"[COMPUTER SYSTEM] Send Speed Modification to Communication: "<<msgToComm.body<<endl;
    msg_struct replyFromComm;
    int status_comm = MsgSend(coid_comm, &msgToComm, sizeof(msgToComm), &replyFromComm, sizeof(replyFromComm));
    if (status_comm == -1) {
       perror("[COMPUTER SYSTEM] MsgSend failed");
       name_close(coid_comm);
       return;
    }

    //cout << "[COMPUTER SYSTEM] Received Reply from Communication: " << replyFromComm.body << endl;
    onetimeBound = 1;
    setBoundCount();
    name_close(coid_comm);
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

       // cout << "[COMPUTER SYSTEM] Received message: " << msgFromOperator.body << endl;

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
                                           to_string(static_cast<int>(ad.speedZ)) + ")" +
                            ", Status: " + (ad.status ? "Active" : "Inactive");
                break;
            }
        }
        if (!found)
            replyBody = "Aircraft not found";

        //send data to display
        int coid_display;
            while (true) {
            	coid_display = name_open("DisplayInfo", 0);
                if (coid_display == -1) {
                    cerr << "Waiting for DisplayInfo server to start..." << endl;
                    sleep(1);
                    continue;
                }
                break;
            }

        msg_struct msgToDisplay;
        msgToDisplay.id = msgFromOperator.id;
        strncpy(msgToDisplay.body, replyBody.c_str(), sizeof(msgToDisplay.body) - 1);
        msgToDisplay.body[sizeof(msgToDisplay.body) - 1] = '\0';

       //cout << "[COMPUTER SYSTEM] Send message To Display: " << msgToDisplay.body << endl;

        msg_struct replyFromDisplay;
           int status_display = MsgSend(coid_display, &msgToDisplay, sizeof(msgToDisplay), &replyFromDisplay, sizeof(replyFromDisplay));
           if (status_display == -1) {
              perror("[COMPUTER SYSTEM] MsgSend failed");
              name_close(coid_display);

           }

          // cout << "[COMPUTER SYSTEM] Received Reply from Communication: " << replyFromDisplay.body << endl;
           name_close(coid_display);

        //Reply to operator
        msg_struct replyToOperator;
        replyToOperator.id = msgFromOperator.id;
        strcpy(replyToOperator.body, "Data Sent to Display");
        if (MsgReply(rcvid, 0, &replyToOperator, sizeof(replyToOperator)) == -1) {
            perror("MsgReply failed in CommandAircraft");
        }

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
    self->ReadData();
    self->CollisionAlerts();

    if(self->getAlertCollision()==false){
    	self->OutofBoundAlerts();

    }

    if(!self->getAlertCollision()&&!self->getAlertOutofBound()){
    	cout<<"NO ALERT"<<endl;
    }

    if (self->getAlertCollision() && self->getOneTimeCollision() == 0){
    	if(self->getCollisionCount()==0){

    		self->SolveCollision();
    		self->IncrementCollisionCount();
    	}

    }
    if (self->getAlertOutofBound() && self->getOneTimeBound() == 0) {
    	if(self->getBoundCount()==0){
    	    		self->RequestCommand();
    	    		self->IncrementBoundCount();
    	}

    }
    // Clear alerts for resolved conditions
        std::set<int> stillOutOfBounds;
        for (const auto& ac : self->aircraftList) {
            bool outOfBounds =
                (ac.x < MIN_X || ac.x > MAX_X ||
                 ac.y < MIN_Y || ac.y > MAX_Y ||
                 ac.z < MIN_Z || ac.z > MAX_Z);
            if (outOfBounds) {
                stillOutOfBounds.insert(ac.id);
            }
        }
        // Remove aircraft that are no longer out of bounds
        for (auto it = self->alertedAircraft.begin(); it != self->alertedAircraft.end();) {
            if (stillOutOfBounds.count(*it) == 0) {
                it = self->alertedAircraft.erase(it);
            } else {
                ++it;
            }
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

    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 1;
    its.it_interval.tv_sec = 1;

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
    pthread_t thread1;

    if (pthread_create(&thread1, NULL, ComputerThread, &cs) != 0) {
        cerr << "Error creating thread" << endl;
        return 1;
    }

    pthread_join(thread1, NULL);
    return 0;
}
