#ifndef DISPLAY_H
#define DISPLAY_H

#include <sys/dispatch.h>
using namespace std;

typedef struct {
    unsigned int id;
    char body[500];
} msg_struct;

class Display {
private:
    name_attach_t* attach = nullptr;

public:
    Display();
    ~Display();
    void StartDisplayServer();
    static void* DisplayServerThread(void* arg);
};

#endif
