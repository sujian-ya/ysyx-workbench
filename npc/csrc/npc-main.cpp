#include <init.h>
#include <sdb.h>

extern void init_npc(int argc, char *argv[]);
extern void sdb_mainloop();
extern void init_sdb();
extern void init_monitor(int argc, char *argv[]);

int main(int argc, char *argv[]) {
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

    sdb_mainloop();
    
    return 0;
}
