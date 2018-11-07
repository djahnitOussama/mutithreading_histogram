#ifndef ZONETEXTE
#define ZONETEXTE
#include <QtWidgets>

class ZoneTexte : public QLabel
{
public:
    ZoneTexte(QWidget *parent = nullptr);
    ZoneTexte(const QString &text, QWidget *parent = nullptr);
    void configuration();
};

#endif // ZONETEXTE
