#ifndef TRACEPICKDIALOG_H
#define TRACEPICKDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <QPushButton>

#include "dataholder.h"

namespace Ui {
class TracePickDialog;
}

//This class provides the methods for the trace selection dialog.
class TracePickDialog : public QDialog
{
    Q_OBJECT

public:
    //Constructor.
    explicit TracePickDialog(const DataHolder &data,
                             int signalNum,
                             QString windowTitle,
                             QWidget *parent = 0);

    //Alt. Constructor.
    explicit TracePickDialog(const DataHolder &data,
                             int signalNum,
                             QString windowTitle,
                             int startRow,
                             int endRow,
                             QWidget *parent = 0,
                             bool multiSelect = true,
                             int *traceNum = nullptr);

    //Destructor.
    ~TracePickDialog();

private slots:
    //Acts if "Ok" is selected.
    void on_buttonBox_accepted();

    //Acts if "Cancel" is selected or the window is closed.
    void on_buttonBox_rejected();

    //Acts if an item on the trace list is changed.
    void on_traceList_itemChanged(QListWidgetItem *item);

    void on_checkBox_toggled(bool checked);

private:
    //Holds the user interface.
    Ui::TracePickDialog *ui;

    //Holds a copy of the dataholder.
    const DataHolder& dataCopy;

    //Holds a copy of the signal number passed on construction.
    int signalNumCopy;

    //Holds a copy of the traces selected.
    QList<bool> traceSelection;

    //Holds copy of row to being reading.
    int startRowCopy;

    //Holds copy of row to end reading.
    int endRowCopy;

    //Holds copy of bool for allowing multiple selections.
    bool multiSelectCopy;

    //Holds reference to the trace used to pick times.
    int* traceNumCopy;

    //Adds signals to the list.
    void addSignals();

    //Prepares the plot to display curves.
    void prepPlot();

    //Prepares and displays curves on the plot
    void prepCurves();

    void prepCurves(int startRow, int endRow);

    //Loads all points from data. VERY SLOW
    QPolygonF loadPrecisePoints(int col, int startRow, int endRow);


    //Loads points from data. Fast, with no visual difference for data with
    //many points.
    QPolygonF loadApproximatePoints(int col, int startRow, int endRow);

signals:

    //Emits the traces selected in the window.
    void tracesSelected(QList<bool>);
};

#endif // TRACEPICKDIALOG_H
