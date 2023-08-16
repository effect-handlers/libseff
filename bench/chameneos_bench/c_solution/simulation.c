/* ===========================================================  */
/*  file simulation .c  */
/* ===========================================================  */
#include  <stdio.h>
#include  <pthread.h>
#include "types.h"
/* ===========================================================  */
colour complementaryColour(colour c1, colour c2){
    if ( c1 == c2)
       return c1;
    else
        return (3 - c1 - c2);
}
/* ===========================================================  */
extern colour Cooperation(idChameneos id, colour c );
extern void initCooperation (void );
/* ===========================================================  */
void chameneosCode(void* args) {
    idChameneos myId;
    colour myColour, oldColour, otherColour ;
    sscanf (( char*  ) args , "%d %d", &myId, &myColour);
    printf ("(%d) I am (%d) and I am running\n", myId, myColour);
    while (1) {
        printf ("(%d) I am (%d) and I am eating honey suckle and training\n", myId, myColour);
        printf ("(%d) I am (%d) and I am going to the mall\n", myId, myColour);
        otherColour = Cooperation( myId, myColour);
        oldColour = myColour;
        myColour = complementaryColour( myColour, otherColour);
        printf ("(%d) I am (%d) and I was %d before\n", myId, myColour, oldColour);
    }
}
/* ===========================================================  */
int main(void) {
    colour tabColour[NB_CHAMENEOS] =  {Yellow, Blue, Red, Blue} ;
    char theArgs[255][NB_CHAMENEOS];
    pthread_t tabPid [NB_CHAMENEOS];
    int i ;
    initCooperation ();
    for ( i=0; i < NB_CHAMENEOS; i++) {
        sprintf ( theArgs[i ], "%d %d", i, tabColour[i ]);
        pthread_create (&tabPid[i ], NULL, (void  *( * )()) chameneosCode, theArgs[i]);
    }
    // waiting the end of children ( that will never come)
    for ( i=0; i < NB_CHAMENEOS; i++) {
        pthread_join(tabPid[i], NULL);
    }
    return 0;
}
