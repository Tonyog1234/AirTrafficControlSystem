#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/dispatch.h>
using namespace std;

typedef struct {
    unsigned int id;
    char body[100];
} msg_struct;
void CommandAircraft(){
	name_attach_t *attach;
	    attach = name_attach(NULL, "myserver", 0);
	    if (attach == NULL) {
	        perror("name_attach");

	    }

	    cout << "Server is running, waiting for messages...\n";

	    while (true) {
	        int rcvid;
	        msg_struct msg;
	        rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);
	        if (rcvid == -1) {
	            perror("MsgReceive failed");
	            continue;
	        }

	        cout << "Received message from ComputerSystem: " << msg.body << endl;
	        cout << "Enter command for Aircraft " << msg.id << ": ";

	        // Get operator input
	        string command;
	        getline(cin, command);  // Use getline to allow spaces in input

	        // Prepare reply
	        msg_struct reply;
	        reply.id = msg.id;
	        strncpy(reply.body, command.c_str(), sizeof(reply.body) - 1);
	        reply.body[sizeof(reply.body) - 1] = '\0';  // Ensure null-terminated

	        // Send reply back to ComputerSystem
	        if (MsgReply(rcvid, 0, &reply, sizeof(reply)) == -1) {
	            perror("MsgReply failed");
	        } else {
	            cout << "Sent reply: " << reply.body << endl;
	        }
	    }

	    name_detach(attach, 0);  // Cleanup (unreachable in this loop)
}
void RequestInfo(){

}
int main() {


    return 0;
}
