#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QListWidgetItem>

#include "dataholder.h"
#include "tracepickdialog.h"

namespace Ui {
class Dialog;
}

//This class provides the methods for the signals and operations selection menu.
class Dialog : public QDialog
{
    Q_OBJECT

public:
    //Constructor.
    explicit Dialog(const DataHolder &data,
                    int signalNum,
                    QWidget *parent = 0);

    Dialog(const DataHolder &data,
           int signalNum,
           int startRow,
           int endRow,
           QWidget *parent = 0);

    //Destructor.
    ~Dialog();

    //Adds signals to boxes.
    void addSignals(QStringList signal);

private slots:
    //Activates when "Ok" is pressed.
    void on_buttonBox_accepted();

    //Activates when "Cancel" is pressed, or the window is closed.
    void on_buttonBox_rejected();

    //Activates when the selection on the signal list changes.
    void on_signalList_itemSelectionChanged();

    //Activates when the selection on the operation list changes.
    void on_operationList_itemSelectionChanged();

    //Activates when the active trace selection is selected, and sets the
    //active traces in the data copy to match.
    void onTracesSelected(QList<bool> activeTraces);

private:
    //Holds the user interface.
    Ui::Dialog *ui;

    //Holds a copy of the DataHolder being operated on.
    DataHolder dataCopy;

    //Holds a copy of the signal number.
    int signalNumCopy;

    //Indicates whether a signal has been selected.
    bool signalSelected;

    //Indicates whether an operation has been selected.
    bool operationSelected;

    int startRowCopy;

    int endRowCopy;

signals:
    //Emits a list of signals selected, operations selected, and the traces
    //being operated on.
    void listIndexes(QList<QListWidgetItem*>,
                     QList<QListWidgetItem*>,
                     QList<bool>);
};

#endif // DIALOG_H
