#ifndef CHRONO_TP_H
#define CHRONO_TP_H

#include <chrono>
using namespace std;

// La bibliothèque propose 3 variantes de chrono (dont l'implémentation est liée au S.E.)
// pour changer la variante remplacer la ligne active par une des 2 en commentaires
typedef  std::chrono::steady_clock    modeChrono;
//typedef  std::chrono::system_clock    modeChrono;
//typedef  std::chrono::high_resolution_clock    modeChrono;


class ChronoTP
{
    chrono::time_point<modeChrono> t0;
    chrono::time_point<modeChrono> t1;
    bool bStop=true;

public:
    ChronoTP()
    {
        start();
        if (uniteDeTemps() != 1e-9)
            qDebug()<< "attention verifier implementation chrono";
        // le message s'affiche si l'unite interne n'est pas la ns
        // il faut introduire une conversion dans lit_ns()
    }

    void start()                // demarrage du chrono
    {
        t0 = modeChrono::now();
        bStop=false;
    }

    double stop_lit_ms()        // arret du chrono, retourne le temps ecoule en millisecondes
    {
        return stop_lit_ns()/1000000.0;
    }

    int64_t stop_lit_us()
    {
        return stop_lit_ns()/1000;
    }

    int64_t stop_lit_ns()           // arret du chrono, retourne le temps ecoule en nanosecondes
    {
        if (!bStop)
        {
            t1= modeChrono::now();
            bStop=true;
        }
        return (t1 - t0).count() ;
    }

    int64_t lit_ns()                // lecture du chrono sans l'arreter, retourne le temps ecoule en nanosecondes
    {
        t1= modeChrono::now();
        return (t1 - t0).count() ;
    }

    double uniteDeTemps()       // verification de l'unite de temps interne
    {
        return modeChrono::period::num/(double)modeChrono::period::den ;
    }

};
#endif // CHRONO_TP_H
