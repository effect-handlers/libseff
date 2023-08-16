all: output/posix output/chameneos_actors output/chameneos_effect_direct output/chameneos_effect_actors

include ../common.mk

output/posix: c_solution/types.h c_solution/simulation.c c_solution/cooperation.c | output
	$(CC) $(CFLAGS) -o output/posix c_solution/simulation.c c_solution/cooperation.c

output/chameneos_actors: chameneos_common.h chameneos_actor.c | output
	$(CC) $(CFLAGS_LIBSEFF) -o output/chameneos_actors.o -c chameneos_actor.c
	$(CC) $(CFLAGS_LIBSEFF) -o output/chameneos_actors output/chameneos_actors.o -lm $(LDFLAGS_LIBSEFF)

output/chameneos_effect_direct: chameneos_common.h chameneos_effect.h chameneos_effect_direct.c | output
	$(CC) $(CFLAGS_LIBSEFF) -o output/chameneos_effect_direct.o -c chameneos_effect_direct.c
	$(CC) $(CFLAGS_LIBSEFF) -o output/chameneos_effect_direct output/chameneos_effect_direct.o -lm $(LDFLAGS_LIBSEFF)

output/chameneos_effect_actors: chameneos_common.h chameneos_effect.h chameneos_effect_actors.c | output
	$(CC) $(CFLAGS_LIBSEFF) -o output/chameneos_effect_actors.o -c chameneos_effect_actors.c
	$(CC) $(CFLAGS_LIBSEFF) -o output/chameneos_effect_actors output/chameneos_effect_actors.o -lm $(LDFLAGS_LIBSEFF)
