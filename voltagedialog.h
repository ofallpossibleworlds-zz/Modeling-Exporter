#ifndef VOLTAGEDIALOG_H
#define VOLTAGEDIALOG_H

#include <QDialog>
#include "dataholder.h"

namespace Ui {
class VoltageDialog;
}

//This class provides the methods for the dialog that manually selects voltages.
class VoltageDialog : public QDialog
{
    Q_OBJECT

public:

    //Constructor (for selection)
    explicit VoltageDialog(const DataHolder & data,
                           QString title,
                           QString helpText,
                           float* voltage,
                           int signalNum,
                           int traceNum,
                           int startRow,
                           int endRow,
                           QWidget *parent = 0);

    //Constructor for time selection.
    explicit VoltageDialog(const DataHolder & data,
                           QString title,
                           QString helpText,
                           float* voltage,
                           int signalNum,
                           int startRow,
                           int endRow,
                           QWidget *parent = 0);

    //
    explicit VoltageDialog(const DataHolder& data,
                           QString title,
                           QString helpText,
                           float* startTime,
                           float* endTime,
                           int signalNum,
                           int startRow,
                           int endRow,
                           int* returnedSignal = nullptr,
                           QWidget *parent = 0);

    //Destructor.
    ~VoltageDialog();

private:
    //Holds the user interface.
    Ui::VoltageDialog *ui;

    //Holds a copy of the dataholder being referenced.
    const DataHolder& dataCopy;

    //Holds a reference to the float that takes voltage/voltage time.
    float* voltageRef;

    float* startTimeCopy;

    float* endTimeCopy;

    int* returnedSignalCopy;

    //A copy of the signal number being referenced.
    int signalNumCopy;

    //A copy of the trace number being referenced.
    int traceNumCopy;

    //A copy of the first row of data to be read.
    int startRowCopy;

    //A copy of the last row of data to be read.
    int endRowCopy;

    //Determines what type of selector to use.
    int selectorTypeCopy;

    //Prepares the plot.
    void prepPlot();

    //Prepares the curves and displays them on the plot.
    void prepCurves();

    void prepCurves(int startRow, int endRow);

    //Loads all points from data. VERY SLOW
    QPolygonF loadPrecisePoints(int col, int startRow, int endRow);

    //Loads points from data. Fast, with no visual difference for data with
    //many points.
    QPolygonF loadApproximatePoints(int col, int startRow, int endRow);

    void setupPlotSelector();

private slots:
    //Acts when a point is selected on the plot representing the
    //intended voltage.
    void voltageSelected(QPointF point);

    void voltageSelected(QRectF rect);

    void on_checkBox_toggled(bool checked);
    void on_plotSelector_currentIndexChanged(int index);
};

#endif // VOLTAGEDIALOG_H
