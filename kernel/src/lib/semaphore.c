#include <lib/semaphore.h>
#include <sys/smp.h>

// TODO: Debug this and find out if this is what is causing my scheduling issues
// when I signal something

void wait(semaphore* s) {
  s->inc--;
  if (s->inc < 0) {
  }
}

void signal(semaphore* s) {
  if (s->inc++ < 0) {
  }
}

void semaphore_init(semaphore* s, i32 init_state) {
  s->inc = init_state;
  s->fidx = 0;
  s->idx = 0;
}