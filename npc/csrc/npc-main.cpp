#include <init.h>
#include <sdb.h>

extern void init_npc(int argc, char *argv[]);
// extern void welcome();
extern void sdb_mainloop();

int main(int argc, char *argv[]) {

    init_npc(argc, argv);
    sdb_mainloop();
    
    return 0;
}
