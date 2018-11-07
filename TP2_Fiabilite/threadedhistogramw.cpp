//                                           TP2 - Master 1 AII - février 2018

#include <thread>
using namespace std;
#include <QPainter>
#include <QtGlobal>
#include <qmath.h>
#include <qdebug.h>

#include "threadedhistogramw.h"
#include "tp2_window.h"
#include "chrono_tp.h"
#include<atomic>
#include <omp.h>
#include<mutex>
#include <QSemaphore>


ThreadedHistogramW::ThreadedHistogramW(int nbLevels, QWidget *parent)
    : QWidget(parent), m_levels(nbLevels)
{
    m_histogram= new int[m_levels]();
    m_initialized = false;
    setMinimumWidth(550);

    // texte et format d'affichage des anomalies du message
    textAnomalies=QString("<br><h4>Scrutateur</h4><pre>")
            +QString(" - Anomalies message :%1<p> - Lignes image<ul>")
            +QString("<li>traitées :       %2<li>non traitées :   %3<li>plusieurs fois : %4</ul><p>");
}

ThreadedHistogramW::~ThreadedHistogramW()
{
    delete[] m_histogram;
}

void ThreadedHistogramW::setLevels(int levels)
{
    if (m_levels == levels)
        return;
    delete[] m_histogram;

    m_histogram= new int[m_levels]();
    m_initialized = false;

    m_levels = levels;
}


void ThreadedHistogramW::setHistogram(int hist[])
{
    for (int i=0; i<m_levels; i++)
        m_histogram[i] = hist[i];
    m_initialized=true;
    update();
}
void ThreadedHistogramW::setHistogram(atomic_int hist[])
{
    // surcharge pour gérer le type atomic_int
    // les cast directs ne sont pas acceptés par le compilateur
    for (int i=0; i<m_levels; i++)
        m_histogram[i] = hist[i];
    m_initialized=true;
    update();
}

void ThreadedHistogramW::paintEvent(QPaintEvent *)
{
    if ( m_initialized != true)
        return;

    // détermination du max, sum et mean
    int sumHist=0;
    float maxHeight=0;
    float meanHeight=0;


    if (m_flagReference)
    {
        // détermination du maximum pour ajuster l'échelle sur l'axe des y
        // les histo supperposés doivent garder la meme échelle (donc meme max)
        for (int i=0; i<m_levels; i++)
        {
            sumHist+=m_histogram[i];
            if (m_histogram[i] > maxHeight)
                maxHeight=m_histogram[i];
        }
        meanHeight=sumHist/float(m_levels);
        referenceMaxH=maxHeight;
        emit signalReferenceData(maxHeight);
    }

    // sélection des couleurs
    QPainter painter(this);
    QPen pen(Qt::black);
    QBrush brush(QColor(200,0,80));
    if (!m_flagReference)
        brush.setColor(QColor(0,00,220,200));
    painter.setBrush(brush);


    // affichage des bins de l'histogramme
    int H1 = 10;
    int H =height()-H1;
    qreal barWidth = width() / float(m_levels);
    qreal deltaIntensite = 255.0/m_levels;
    for (int i = 0; i < m_levels; i++)
    {
        qreal h = m_histogram[i]/referenceMaxH * H;
        qreal intensite = deltaIntensite*i;
        // draw level
        painter.fillRect(barWidth * i, H - h, barWidth -1, h, brush);
        // draw témoin niveau de gris
        painter.fillRect(barWidth * i, H+1 , barWidth -1, H1-1, QColor(intensite,intensite,intensite));
    }

    if (m_flagReference)
    {
        painter.setPen(pen);
        qreal h = meanHeight/referenceMaxH * H;
        painter.drawLine(0,H-h,width(),H-h);
        painter.drawText(0,H-h-2,"moyenne");
    }
}


void ThreadedHistogramW::scanImageMono(QImage *img)
{
    int H = img->height();
    int W = img->width();

    int * localHistogramme=new int[m_levels]();

    for(int y = 0; y < H; y++)      // pour chaque ligne de l'image
    {
        QRgb* tabLigne = (QRgb*)img->constScanLine(y); // pointeur sur début de ligne

        for(int x = 0; x < W; x++)  // pour chaque pixel de la ligne
        {
            // Conversion du pixel en Niveau de gris = somme pondérée des composantes couleur
            float ng= qRed(tabLigne[x])*coefRGB[0]+ qGreen(tabLigne[x])*coefRGB[1]+qBlue(tabLigne[x])*coefRGB[2] ;

            // détermine l'indice dans l'histogramme
            int bin=qFloor(ng/255*(m_levels-1));

            // par sécurité, on vérifie qu'il n'y a pas de débordement des valeurs
            bin = max(bin,0);
            bin = min(bin,m_levels-1);
            // incrémentation du bin correspondant;
            localHistogramme[bin]++;

        }
    }
    setHistogram(localHistogramme);
    delete[] localHistogramme;
}




void ThreadedHistogramW::scanImageMulti(QImage *img)
{
    // tableau pour histogramme commun pour tous les threads
    int* histo = new int[m_levels];               // variable partagée

    memset(histo,0,sizeof(*histo)*m_levels);      // initialisation du tableau à 0

    // démarrage du chrono
    ChronoTP monChrono;

    // création des threads
    std::thread th[nbreThread];
    for (int i=0;i<nbreThread;i++)
    {
        th[i]=std::thread(&ThreadedHistogramW::scanImagePartiel,this, img, i, histo);
    }
    bool flagStop=false;
    std::thread thScrut(&ThreadedHistogramW::scrutateur,this,img,&flagStop);

    // attente fin de tous les threads
    // puis on signale le flag et attend fin scrutateur
    for (int i=0;i<nbreThread;i++)
        th[i].join();
    flagStop=true;
    thScrut.join();

    // tous les threads ont complété le meme histo, il est prêt à afficher
    setHistogram(histo);

    // affichage de la durée
    float duree = monChrono.stop_lit_ms();
    emit signalDuree(QString("durée %1 ms").arg(duree));

    QString message=QString("taille %1x%2 pixels   durée %3 ms").
            arg(img->width()).arg(img->height()).arg(duree) ;
    qDebug()<<message;

    // vérification histogramme et affichage
    int sommeHist=0;
    for (int i=0; i<m_levels;i++)
        sommeHist+=histo[i];
    int diff=  img->height()*img->width()- sommeHist;   // somme espérée - somme observée
    QString filename = "etude2c1.ods";
    QFile file(filename);
    if (file.open(QIODevice::ReadWrite | QIODevice::Append)) {
     QTextStream stream(&file);

    stream << diff << endl;
    file.close();
    }
    QString st("<h4>Histogramme</h4><pre> - Différence sommes :%1   ");
    emit signalAnomalie1(st.arg(diff,7));
}

dataL messageData;   // variable globale pour communiquer entre threads
QSemaphore sem1(1); //variable globale qsemaphore
QSemaphore sem2(0);
void ThreadedHistogramW::scanImagePartiel(QImage *img, int ligne0, int *histo)
{
    int H = img->height();
    int W = img->width();
    #pragma omp parallel for num_threads(2)
    for(int y = ligne0; y< H ; y+=nbreThread)      // pour chaque ligne de l'image
    {
        QRgb* tabLigne = (QRgb*)img->constScanLine(y); // pointeur sur début de ligne

        for(int x = 0; x < W; x++)  // pour chaque pixel de la ligne
        {
            // Conversion du pixel en Niveau de gris = somme pondérée des composantes couleur
            float ng= qRed(tabLigne[x])*coefRGB[0]+ qGreen(tabLigne[x])*coefRGB[1]+qBlue(tabLigne[x])*coefRGB[2] ;

            // détermine l'indice dans l'histogramme
            int bin=qFloor(ng/255*(m_levels-1));

            // par sécurité, on vérifie qu'il n'y a pas de débordement des valeurs
            bin = max(bin,0);
            bin = min(bin,m_levels-1);

            // incrémentation du bin correspondant;
            m1.lock();
            histo[bin]++;
            m1.unlock();

        }

        // message pour scrutateur = indication de la ligne traitée, écart-type
        sem1.acquire();
         messageData.indiceLigneBis=y;
         double somCarree=0;
         for (int i=0; i<tailleHistogram;i++)
             somCarree+=pow((double)histo[i],2);      // puissance 2
         messageData.ecartType=sqrt(somCarree/tailleHistogram);
         messageData.indiceLigne=y;
         sem2.release();
    }
}

void ThreadedHistogramW::scrutateur(QImage *img, volatile bool *flagStop)
{
    int nbrIncoherencesM=0;            // compteurs
    int nbrVal=0;
    int nbLignesPerdues=0;
    int nbLignesMulti=0;

    int precedent=-1;               // indice ligne courante

    int nbLignes=img->height();     // tableau pour noter lignes traitées par thread
    int *tabLignes = new int[nbLignes];
    memset(tabLignes,0,sizeof(int)*img->height());

    while (! *flagStop)             // attente fin des autres threads
    {
        if (precedent==messageData.indiceLigne)
            continue;
        sem2.acquire();
        precedent=messageData.indiceLigne;

        // vérifie que toutes les lignes de l'image ont bien été scannées
        nbrVal++;
        tabLignes[precedent]++;

        // véfication cohérence dataL
        if (messageData.indiceLigneBis != messageData.indiceLigne)
            nbrIncoherencesM++;
        sem1.release();
    }
    for (int i=0; i< nbLignes; i++)
    {
        if (tabLignes[i]==0)
            nbLignesPerdues++;
        if (tabLignes[i]>1)
            nbLignesMulti++;
    }

    int nFormat=7;
    emit signalAnomalies2(textAnomalies.arg(nbrIncoherencesM,nFormat).arg(nbrVal,nFormat).arg(nbLignesPerdues,nFormat).arg(nbLignesMulti,nFormat));
}
