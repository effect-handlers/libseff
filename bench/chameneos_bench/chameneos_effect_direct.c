#include "chameneos_effect.h"

void execute(size_t meetings, colour *creatures, size_t creaturesSize) {
    // Print the colours
    for (int i = 0; i < creaturesSize; ++i) {
        printf("%s ", colourNames[creatures[i]]);
    }
    printf("\n");

    seff_coroutine_t **chms = calloc(creaturesSize, sizeof(seff_coroutine_t *));
    seff_request_t *reqs = calloc(creaturesSize, sizeof(seff_request_t));
    chameneos_init_effect_t *inits = calloc(creaturesSize, sizeof(chameneos_init_effect_t));

    for (size_t i = 0; i < creaturesSize; ++i) {
        inits[i].self = i;
        inits[i].col = creatures[i];
        chms[i] = seff_coroutine_new(chameneos, (void *)&inits[i]);
        // First resume, we assume it's not expecting anything
        reqs[i] = seff_resume(chms[i], (void *)NULL, HANDLES(meet));
    }

    // This takes advantage of this problem structure, not really fair, but for experimental
    // purposes
    size_t next = 0;

    for (size_t i = 0; i < meetings; i++) {
        size_t a = next;
        next = (next + 1) % creaturesSize;
        uint64_t chameneosA;
        colour colA;

        size_t b = next;
        next = (next + 1) % creaturesSize;
        uint64_t chameneosB;
        colour colB;

        switch (reqs[a].effect) {
            CASE_EFFECT(reqs[a], meet, {
                chameneosA = payload.self;
                colA = payload.msg;
            })
        };

        switch (reqs[b].effect) {
            CASE_EFFECT(reqs[b], meet, {
                chameneosB = payload.self;
                colB = payload.msg;
            })
        };

        chameneos_meet_t meetMsg;

        meetMsg.finish = false;
        meetMsg.chameneos = chameneosB;
        meetMsg.col = colB;
        reqs[a] = seff_resume(chms[a], (void *)&meetMsg, HANDLES(meet));

        meetMsg.finish = false;
        meetMsg.chameneos = chameneosA;
        meetMsg.col = colA;
        reqs[b] = seff_resume(chms[b], (void *)&meetMsg, HANDLES(meet));
    }

    uint64_t total_meetings = 0;
    for (size_t i = 0; i < creaturesSize; ++i) {
        chameneos_meet_t meetMsg;
        meetMsg.finish = true;
        // First resume
        total_meetings += (uint64_t)seff_resume(chms[i], (void *)&meetMsg, HANDLES(meet)).payload;
    }

    spell_int(total_meetings);

    return;
}

int main() {
    print_complements();
    printf("\n");

    execute(600, firstCreatures, firstCreaturesSize);
    printf("\n");

    execute(600, secondCreatures, secondCreaturesSize);
    printf("\n");

    return 0;
}
