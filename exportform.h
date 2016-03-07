#ifndef EXPORTFORM_H
#define EXPORTFORM_H

#include <QDialog>

namespace Ui {
class ExportForm;
}

class ExportForm : public QDialog
{
    Q_OBJECT

public:
    explicit ExportForm(int *voltage,
                        int *index,
                        QString *title,
                        QWidget *parent = 0);
    ~ExportForm();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_nameSelector_textChanged(const QString &arg1);

    void on_variantSelector_valueChanged(int arg1);

    void on_voltageSelectorBox_valueChanged(int arg1);

private:
    Ui::ExportForm *ui;

    int* voltageHolder;

    int* indexHolder;

    QString* titleHolder;
};

#endif // EXPORTFORM_H
