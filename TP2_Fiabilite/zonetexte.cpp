#include <QtWidgets>
#include "zonetexte.h"

ZoneTexte::ZoneTexte(QWidget *parent )
: QLabel(parent)
{
    configuration();
}

ZoneTexte::ZoneTexte(const QString &text, QWidget *parent)
: QLabel(text,parent)
{
    configuration();
}

void ZoneTexte::configuration()
{
    setFixedHeight(30);
    QFont font("Helvetica",10);
    setFont(font);
}
