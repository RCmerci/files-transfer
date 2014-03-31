///////
#include "multhread_trans.h"
extern recvfd;
int main(int argc, char **argv){
    port = atoi(argv[1]);
    init();
    loop();
    return 0;
}
