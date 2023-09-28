#include "actors.h"
#include "seff.h"

typedef struct {
    actor_t *sender;
    int64_t number;
} pong_msg;

#define GUARD(mutex, block)                \
    {                                      \
        pthread_mutex_lock(mutex);         \
        block pthread_mutex_unlock(mutex); \
    }
static pthread_mutex_t puts_mutex;

#define threadsafe_puts(str) GUARD(&puts_mutex, { puts(str); });
#define threadsafe_printf(...) GUARD(&puts_mutex, { printf(__VA_ARGS__); })

void *pong_fn(seff_coroutine_t *k, actor_t *self) {
    threadsafe_puts("[pong] initialized");

    while (true) {
        pong_msg *msg = actor_recv(self);
        if ((int64_t)msg == -1) {
            threadsafe_puts("[pong] received kill request");
            break;
        } else {
            threadsafe_puts("[pong] increasing number");
            assert(msg->number + 1 != 0);
            actor_send(msg->sender, (void *)(msg->number + 1));
        }
    }

    threadsafe_puts("[pong] finished");
    return NULL;
}

void *ping_fn(seff_coroutine_t *k, actor_t *self) {
    threadsafe_puts("[ping] initialized");
    threadsafe_puts("[ping] awaiting pong address");

    actor_t *pong = actor_recv(self);
    threadsafe_puts("[ping] received pong address");

    int64_t i = 0;
    while (i < 5000) {
        pong_msg msg = {self, i};
        threadsafe_puts("[ping] sending number");
        actor_send(pong, &msg);
        i = (int64_t)actor_recv(self);
    }
    threadsafe_puts("[ping] sending finalization");
    actor_send(pong, (void *)-1);
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

int main(void) {
    actor_start(actors_main, NULL, THREADS, true);
    return 0;
}
