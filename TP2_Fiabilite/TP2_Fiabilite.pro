#-------------------------------------------------
#
# Project TP2 - Master 1 AII - février 2018
#-------------------------------------------------

QT +=  widgets

# on impose que l'executable soit dans le repertoire du projet
# pour eviter la recherche du du dossier /img
DESTDIR= $$_PRO_FILE_PWD_

TARGET = TP2_Histogram
TEMPLATE = app
CONFIG += c++11

SOURCES +=\
    tp2_main.cpp \
    tp2_window.cpp \
    zonetexte.cpp \
    threadedhistogramw.cpp

HEADERS  +=\
    chrono_tp.h \
    tp2_window.h \
    zonetexte.h \
    threadedhistogramw.h

QMAKE_CXXFLAGS +=-fopenmp
QMAKE_LFLAGS += -fopenmp



