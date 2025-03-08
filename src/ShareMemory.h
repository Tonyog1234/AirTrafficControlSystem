#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <semaphore.h>

// Constants for airspace dimensions
const int AIRSPACE_WIDTH = 100000;
const int AIRSPACE_HEIGHT = 25000;
const int AIRSPACE_LENGTH = 100000;
const int MIN_SEPARATION_XY = 3000;
const int MIN_SEPARATION_Z = 1000;
const int MAX_AIRCRAFT = 5;  // Max aircraft in airspace

// Aircraft Data Structure
struct AircraftData {
    int id;
    float x, y, z;         // Position
    float speedX, speedY, speedZ; // Speed in each direction
    bool active;           // Whether the aircraft is active
};

// Shared Memory Structure
struct SharedAirspace {
    AircraftData aircraft[MAX_AIRCRAFT]; // Array of aircraft data
    sem_t semaphore;  // Semaphore for synchronization
};

#endif // SHARED_MEMORY_H
