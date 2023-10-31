#pragma once

#include "chameneos_common.h"
#include "seff.h"

typedef struct {
    bool finish;
    uint64_t chameneos;
    colour col;
} chameneos_meet_t;

DEFINE_EFFECT(meet, 21, chameneos_meet_t *, {
    uint64_t self;
    colour msg;
});

typedef struct {
    uint64_t self;
    colour col;
} chameneos_init_effect_t;

void *chameneos(void *arg) {
    // receive initial info
    chameneos_init_effect_t *info = (chameneos_init_effect_t *)arg;
    colour self_col = info->col;
    uint64_t self_id = info->self;

    uintptr_t meetings = 0;
    int self_meetings = 0;

    // loop
    while (1) {
        //  send meet request and receive mate
        chameneos_meet_t *mate = PERFORM(meet, self_id, self_col);

        if (mate->finish) {
            // received end message
            break;
        }

        //  complement colour
        self_col = complement(self_col, mate->col);
        meetings++;
        if (self_id == mate->chameneos) {
            // this shoul never happen
            self_meetings++;
        }
    }

    // print counters
    printf("%lu ", meetings);
    spell_int(self_meetings);

    return (void *)meetings;
}
