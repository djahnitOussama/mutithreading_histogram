//                                           TP2 - Master 1 AII - février 2018

//#include <iostream>
#include <thread>
#include <bitset>
using namespace std;

#include <QtWidgets>

#include "zonetexte.h"
#include "chrono_tp.h"
#include "tp2_window.h"
#include "threadedhistogramw.h"

// Paramètres à adapter durant le TP
const QString nomImageInitiale="img/Stonehenge.jpg";
const QSize refSize(250, 250);      // taille du bouton-image


TP2_Window::TP2_Window()
    :QMainWindow()
{
     setWindowTitle("Histogramme d'intensité");

     // creation du bouton de selection d'images
    boutonImage = new QToolButton;
    boutonImage->setIconSize(refSize);
    boutonImage->setToolTip("clic pour selectionner une autre image");
    connect(boutonImage, &QToolButton::clicked, this, &TP2_Window::selectImage);


    // creation de la zone d'affichage de l'histogramme
    affichRefHisto = new ThreadedHistogramW(tailleHistogram);
    affichRefHisto->setReference();
  //  int temp[tailleHistogram]{2500};
    //affichRefHisto->setHistogram(temp);
    affichHisto = new ThreadedHistogramW(tailleHistogram);

    // initialisation des zones de textes
    titreImage=new ZoneTexte("Image");
    QString texte=QString("Histogramme en Niveaux de Gris (%1 bins)").arg(tailleHistogram);
    titreHisto=new ZoneTexte(texte);
    infoImage = new ZoneTexte("taille image");
    infoAlgo = new ZoneTexte("durée calcul histogramme");
    titreAnomalies = new ZoneTexte("<h3>Anomalies observées</h3>");

    // organisation de la fenetre principale en 3 colonnes
    QHBoxLayout *mainLayout=new QHBoxLayout;
    QVBoxLayout *leftLayout=new QVBoxLayout;
    QVBoxLayout *centralLayout=new QVBoxLayout;
    QVBoxLayout *rightLayout=new QVBoxLayout;
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(centralLayout);
    mainLayout->addLayout(rightLayout);
    setCentralWidget(new QWidget) ;
    centralWidget()->setLayout( mainLayout);

    // présentation de la colonne de gauche
    leftLayout->addWidget(titreImage);
    leftLayout->addWidget(boutonImage);
    leftLayout->addStretch();
    leftLayout->addWidget(infoImage);
    infoImage->setMaximumWidth(boutonImage->width());

    // colonne centrale
    centralLayout->addWidget(titreHisto);
    titreHisto->setMaximumHeight(20);
    QStackedLayout * stackedLayout= new QStackedLayout;
    stackedLayout->addWidget(affichRefHisto);
    stackedLayout->addWidget(affichHisto);
    stackedLayout->setStackingMode(QStackedLayout::StackAll);
    centralLayout->addLayout(stackedLayout);
    centralLayout->addWidget(infoAlgo);

    // colonne de droite : anomalies
    rightLayout->addWidget(titreAnomalies);
    QLabel * infoErr1=new QLabel("<h4>Histogramme</h4>Anomalie sur somme");
    QLabel * infoErr2=new QLabel("<h4>Scrutateur</h4>");
 //   QLabel * infoErr3=new QLabel("Anomalie type 3 ");
    rightLayout->addWidget(infoErr1);
    rightLayout->addWidget(infoErr2);
   // rightLayout->addWidget(infoErr3);

    // colonne de droite : options de configuration
    // vérification de la sélection effective des coeurs
    QString textConfig("<h2>Configuration exécution</h2>");
    textConfig.append(QString("<pre> - Nombre de threads : %1").arg(nbreThread));

    // vérification nombre processeurs
    unsigned int nCoeurs = std::thread::hardware_concurrency();

#ifndef _WIN32   // nécessaire pour linux à tester sur MacOS
    cpu_set_t my_set;
    memset(&my_set,0,sizeof(my_set));
    sched_getaffinity(0, sizeof(my_set), &my_set);
    nCoeurs = CPU_COUNT(&my_set);
#endif

    if (nCoeurs)
        textConfig.append(QString("<pre> - Coeurs actifs :     %1").arg(nCoeurs));
    else
        textConfig.append("Coeurs actifs : non disponible");


    QLabel * infoConfig=new QLabel(textConfig);
    rightLayout->addWidget(infoConfig);
    rightLayout->addStretch();

    // menu
    QMenu * menu1=new QMenu("Actions");
    menu1->addAction("Ouvrir Image",this, SLOT(selectImage()),QKeySequence("o"));
    menu1->addAction("Recalcule Histo",this, &TP2_Window::recalculHisto,QKeySequence("r"));
    menu1->addAction("Quitter",qApp, SLOT(quit()),QKeySequence(QKeySequence::Quit));
    menuBar()->addMenu(menu1);

    // interactions du widget histogramme
    connect(affichHisto,&ThreadedHistogramW::signalDuree,infoAlgo,&ZoneTexte::setText);
    connect(this,&TP2_Window::ImageChargee,affichRefHisto,&ThreadedHistogramW::scanImageMono);
    connect(this,&TP2_Window::ImageChargee,affichHisto,&ThreadedHistogramW::scanImageMulti);
    connect(affichRefHisto,&ThreadedHistogramW::signalReferenceData,affichHisto,&ThreadedHistogramW::ReferenceData);

    connect(affichHisto,&ThreadedHistogramW::signalAnomalie1,infoErr1,&QLabel::setText);
    connect(affichHisto,&ThreadedHistogramW::signalAnomalies2,infoErr2,&QLabel::setText);
    // chargement image initiale
    loadImage(nomImageInitiale, &image1, boutonImage);
}


void TP2_Window::selectImage()
{
    // Ouverture boite de dialogue standard pour sélection
    QString fileName = QFileDialog::getOpenFileName(this, "Choisir image","img");
    if (fileName.isEmpty())
        return;

    loadImage(fileName, &image1, boutonImage);
}

void TP2_Window::recalculHisto()
{
   emit ImageChargee(&image1);
}


int TP2_Window::loadImage(const QString &fileName, QImage *image,
                           QToolButton *button)
{
    image->load(fileName);
    if (image->isNull())
    {
        qDebug()<<"échec chargement image "<<fileName;
        return -1;
   }

    // Changement d'échelle (respectant les proportions de l'image)
   *image=image->scaledToHeight(image->height()*coefAgrandissement);

    // affichage du bouton
    button->setIcon(QPixmap::fromImage(*image));
    if (image->format()!=QImage::Format_RGB32)  // pour image en niveaux de gris
        *image=image->convertToFormat(QImage::Format_RGB32);

    // affichage des informations
    QFileInfo fInfos(fileName);
    QString texte1=QString("Image <i>%1</i>").arg(fInfos.fileName());
    titreImage->setText(texte1);
    QString texte2=QString("Taille %1x%2 pixels (coef x%3)").
            arg(image1.width()).arg(image1.height()).arg(coefAgrandissement);
    infoImage->setText(texte2);

    emit ImageChargee(image);
    return 0;
}

