#if defined(COMPILE_COCKPIT)
#include "cockpit/controller.c"
#elif defined(COMPILE_BLINKER)
#include "blinker/controller.c"
#else
#error "You need to define COMPILE_COCKPIT or COMPILE_BLINKER"
#endif

int main(void) { main_loop(); }
