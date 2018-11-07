//                                           TP2 - Master 1 AII - février 2018

#include "tp2_window.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    // gestion du masque d'affinité
#ifdef WIN32
    SetProcessAffinityMask(GetCurrentProcess(),masqueAffinite); // Mode monoprocesseur, biprocesseur ou multiprocesseur
#else
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    cpuset.__bits[0]=masqueAffinite;
    sched_setaffinity(0, sizeof(cpuset), (cpu_set_t *)&cpuset);
#endif

    QApplication app(argc, argv);
    TP2_Window w;
    w.show();

    return app.exec();
}
