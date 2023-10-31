#include "actors.h"
#include "chameneos_common.h"

typedef struct {
    actor_t *broker;
    colour col;
} chameneos_init_t;

typedef struct {
    actor_t *chameneos;
    colour col;
} chameneos_meet_t;

typedef union {
    int final_meetings;
    chameneos_meet_t mating;
} broker_message_options;

typedef struct {
    bool finish;
    broker_message_options payload;
} broker_message_t;

void *chameneos(struct actor_t *self) {
    // receive initial info
    chameneos_init_t *info = actor_recv(self);
    actor_t *broker = info->broker;
    colour self_col = info->col;
    free(info);

    int meetings = 0;
    int self_meetings = 0;

    // loop
    while (1) {
        //  send meet request
        broker_message_t *msg = malloc(sizeof(broker_message_t));
        msg->finish = false;
        msg->payload.mating.chameneos = self;
        msg->payload.mating.col = self_col;
        actor_send(broker, msg);

        //  receive mate or end (1)
        chameneos_meet_t *mate = actor_recv(self);
        if (mate == (chameneos_meet_t *)1) {
            // received end message
            break;
        }

        //  complement colour
        self_col = complement(self_col, mate->col);
        meetings++;
        if (self == mate->chameneos) {
            // this should never happen
            self_meetings++;
        }
        free(mate);
    }

    // print counters
    printf("%d ", meetings);
    spell_int(self_meetings);

    // send final counter and finish
    broker_message_t *msg = malloc(sizeof(broker_message_t));
    msg->finish = true;
    msg->payload.final_meetings = meetings;
    actor_send(broker, msg);

    return NULL;
}

typedef struct {
    colour *creatureColours;
    size_t numberOfCreatures;
    size_t meetings;
} broker_init_t;

void *broker_fn(struct actor_t *self) {
    broker_init_t *init = actor_recv(self);

    int meetings = init->meetings;
    size_t creatures = init->numberOfCreatures;
    colour *creatureColours = init->creatureColours;
    free(init);

    // Print the colours
    for (int i = 0; i < creatures; ++i) {
        printf("%s ", colourNames[creatureColours[i]]);
    }
    printf("\n");

    // spawn chameneos and send first info
    actor_t **actors = calloc(creatures, sizeof(actor_t *));
    for (int i = 0; i < creatures; i++) {
        actors[i] = fork_actor(chameneos);
        chameneos_init_t *msg = malloc(sizeof(chameneos_init_t));
        msg->broker = self;
        msg->col = creatureColours[i];
        actor_send(actors[i], (void *)msg);
    }

    // Mate pairs
    for (int i = 0; i < meetings; i++) {
        broker_message_t *first = actor_recv(self);
        assert(!first->finish);
        chameneos_meet_t *first_message = malloc(sizeof(chameneos_meet_t));
        *first_message = first->payload.mating;

        broker_message_t *second = actor_recv(self);
        assert(!second->finish);
        chameneos_meet_t *second_message = malloc(sizeof(chameneos_meet_t));
        *second_message = second->payload.mating;

        actor_send(first->payload.mating.chameneos, second_message);
        actor_send(second->payload.mating.chameneos, first_message);
        free(first);
        free(second);
    }

    // Finalize chameneos
    for (int i = 0; i < creatures; i++) {
        actor_send(actors[i], (void *)1);
    }

    // wait till all done
    int remaining = creatures;
    int total_meetings = 0;
    while (remaining) {
        broker_message_t *ans = actor_recv(self);
        if (ans->finish) {
            remaining--;
            total_meetings += ans->payload.final_meetings;
        }
        free(ans);
    }

    spell_int(total_meetings);

    free(actors);

    return NULL;
}

void *actors_main(void *arg) {
    actor_t *broker = fork_actor(broker_fn);
    actor_insert_msg(broker, arg);

    return NULL;
}

#define THREADS 1
#define TASK_QUEUE_SIZE 128

void execute_chameneos(void *arg) { actor_start(actors_main, arg, THREADS, true); }

int main(void) {
    // show complements
    print_complements();
    printf("\n");

    {
        broker_init_t *init = malloc(sizeof(broker_init_t));
        init->creatureColours = firstCreatures;
        init->numberOfCreatures = firstCreaturesSize;
        init->meetings = 600;

        execute_chameneos(init);
    }
    printf("\n");

    {
        broker_init_t *init = malloc(sizeof(broker_init_t));
        init->creatureColours = secondCreatures;
        init->numberOfCreatures = secondCreaturesSize;
        init->meetings = 600;

        execute_chameneos(init);
    }
}
