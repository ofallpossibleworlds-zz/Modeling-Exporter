#ifndef TIMESELECTOR_H
#define TIMESELECTOR_H

#include <QDialog>

namespace Ui {
class TimeSelector;
}

class TimeSelector : public QDialog
{
    Q_OBJECT

public:
    explicit TimeSelector(QWidget *parent = 0);
    ~TimeSelector();

private:
    Ui::TimeSelector *ui;
};

#endif // TIMESELECTOR_H
