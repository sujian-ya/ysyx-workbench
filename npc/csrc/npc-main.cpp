#include <init.h>
#include <sdb.h>

extern void init_npc(int argc, char *argv[]);
extern void sdb_mainloop();
extern void init_sdb();

int main(int argc, char *argv[]) {

    init_npc(argc, argv);
    init_sdb();
    sdb_mainloop();
    
    return 0;
}
