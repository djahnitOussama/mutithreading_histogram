#ifndef THREADEDHIST_H
#define THREADEDHIST_H

#include <QWidget>
#include <thread>
using namespace std;
#include "tp2_window.h"
#include<atomic>
#include<mutex>
#include<qsemaphore.h>
struct dataL{
    int indiceLigne;
    double ecartType;
    int indiceLigneBis;
};


class ThreadedHistogramW
        : public QWidget
{
    Q_OBJECT
private:
    mutex m1;
    int m_levels;
    int* m_histogram;
    bool m_initialized;
    bool m_flagReference=false;
    float referenceMaxH=0;
    QString textAnomalies;

    // pour la conversion RGB -> niveau de gris, les coefficients généralement utilisés
    // sont  0.2989 * R + 0.5870 * G + 0.1140 * B
    float coefRGB[3]={0.2989, 0.5870, 0.1140};

public:
    ThreadedHistogramW(int nbLevels, QWidget *parent = nullptr);
    ~ThreadedHistogramW();
    void setLevels(int levels);
    void setReference(bool b=true)  { m_flagReference=b; }

signals:
    void signalDuree(QString);
    void signalReferenceData(float maxH);
    void signalAnomalie1(QString);
    void signalAnomalies2(QString);

protected:
    void paintEvent(QPaintEvent *event);

    void scanImageOMP(QImage *img);
    void scanMultiThread(QImage *img);
    void scanImagePartiel(QImage *img, int ligne0, int *histo);
    void scrutateur(QImage *img, volatile bool *);

public slots:
    void setHistogram(int hist[]);
    void setHistogram(atomic_int hist[]);   // surcharge pour version TP2
    void scanImageMulti(QImage *img);
    void scanImageMono(QImage *img);
    void ReferenceData(float maxH) {referenceMaxH=maxH;}


};


#endif // THREADEDHIST_H
