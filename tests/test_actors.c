#include "actors.h"
#define SCHEDULER_POLICY_WORK_SPILLING
#include "scheduler.c"

typedef struct {
    actor_t *sender;
    int64_t number;
} pong_msg;

void *pong_fn(seff_coroutine_t *k, actor_t *self) {
    threadsafe_puts("[pong] initialized");

    while (true) {
        pong_msg *msg = recv(self);;
        if ((int64_t)msg == -1) {
            threadsafe_puts("[pong] received kill request");
            break;
        } else {
            threadsafe_puts("[pong] increasing number");
            assert(msg->number + 1 != 0);
            send(msg->sender, (void *)(msg->number + 1));
        }
    }

    threadsafe_puts("[pong] finished");
    return NULL;
}

void *ping_fn(seff_coroutine_t *k, actor_t *self) {
    threadsafe_puts("[ping] initialized");
    threadsafe_puts("[ping] awaiting pong address");

    actor_t *pong = recv(self);
    threadsafe_puts("[ping] received pong address");

    int64_t i = 0;
    while (i < 5000) {
        pong_msg msg = {self, i};
        threadsafe_puts("[ping] sending number");
        send(pong, &msg);
        i = (int64_t)recv(self);
    }
    threadsafe_puts("[ping] sending finalization");
    send(pong, (void *)-1);
    threadsafe_puts("[ping] finished");

    return NULL;
}

void *actors_main(seff_coroutine_t *self, void *arg) {
    threadsafe_puts("Hello from actors_main");

    actor_t *ping = fork_actor(ping_fn);
    threadsafe_puts("created ping");
    actor_t *pong = fork_actor(pong_fn);
    threadsafe_puts("created pong");
    threadsafe_puts("telling ping about pong");
    actor_insert_msg(ping, pong);
    threadsafe_puts("green thread done");

    return NULL;
}

#define THREADS 4
#define TASK_QUEUE_SIZE 128

int main(void) {
    scheduler_t s;
    scheduler_init(&s, THREADS, TASK_QUEUE_SIZE);

    scheduler_schedule(&s, actors_main, NULL, 0);
    scheduler_start(&s);

    threadsafe_puts("main idling");

    scheduler_join(&s);
    scheduler_destroy(&s);
}
