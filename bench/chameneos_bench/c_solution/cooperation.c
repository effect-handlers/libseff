/*  ===========================================================  */
/*  file cooperation .c  */
/*  ===========================================================  */
#include "types.h"
#include <semaphore.h>

sem_t AtMostTwo;
sem_t Mutex;
sem_t SemPriv;

int FirstCall = 1;
colour AColour;
colour BColour;

/*  ===========================================================  */
colour Cooperation(idChameneos id, colour c) {
    colour otherColour;

    sem_wait(&AtMostTwo); // limits the number of partners
    sem_wait(&Mutex);
    // user programmed mutual exclusion = setting the lock
    if (FirstCall) {
        AColour = c;
        FirstCall = 0;
        // the next call will be considered as a second one
        sem_post(&Mutex);
        sem_wait(&SemPriv); // waiting for the lock

        otherColour = BColour;
        sem_post(&Mutex); // releases the lock since the rendez-vous ends
        sem_post(&AtMostTwo);
        sem_post(&AtMostTwo); // allows a new pair
    } else {                  // this is the second chameneos of the pair
        FirstCall = 1;
        BColour = c;
        otherColour = AColour;
        // the next call will start a new meeting
        sem_post(&SemPriv); // passes the lock to its mate
    }
    return otherColour;
}
/* ===========================================================  */
void initCooperation(void) {
    sem_init(&AtMostTwo, 0, 2);
    sem_init(&Mutex, 0, 1);
    sem_init(&SemPriv, 0, 0);
}
