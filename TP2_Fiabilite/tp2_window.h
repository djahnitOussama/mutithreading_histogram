#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Configuration de l'exécution  - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const int masqueAffinite = 0xFF;
                    //valeurs de masque =  1 ==> ordonnancement des threads sur un seul processeur
                    //					   5 ==> exécution des threads sur 2 processeurs,
                    //					   FF ==> exécution des threads sur tous les processeurs,
                    //   remarque à vérifier en fonction de votre machine

const int nbreThread = 8;               // nombre de t
const float coefAgrandissement = 4;     // facteur d'agrandissment appliqué à l'image avant
                                        // réalisation de l'histogramme

const int tailleHistogram = 1;        // nombre de colonnes de l'histogramme
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


#include "zonetexte.h"
#include "threadedhistogramw.h"

class ThreadedHistogramW;

class TP2_Window : public QMainWindow
{
    Q_OBJECT

    QToolButton *boutonImage;
    ThreadedHistogramW *affichHisto;
    ThreadedHistogramW *affichRefHisto;
    QImage image1;

    ZoneTexte * titreImage;
    ZoneTexte * titreHisto;
    ZoneTexte * infoImage;
    ZoneTexte * infoAlgo;
    ZoneTexte * titreAnomalies;

    float duree=0;

public:
    TP2_Window();       // constructeur de la classe - organisation de la fenetre principale

private:
    void chooseImage(const QString &title, QImage *image, QToolButton *button);
    int loadImage(const QString &fileName, QImage *image, QToolButton *button);

signals:
    void ImageChargee(QImage *img);

private slots:
    void selectImage();
    void recalculHisto();
};

#endif // MAINWINDOW_H
