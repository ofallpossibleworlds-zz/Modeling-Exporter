#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QInputDialog>
#include <qwt_plot.h>
#include "dataholder.h"
#include "dialog.h"
#include "voltagedialog.h"
#include "exportform.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    const QString EXPORT_VARIABLE = "A";

public:
    //Constructor
    explicit MainWindow(QWidget *parent = 0);

    //Destructor
    ~MainWindow();



private:

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////VARIABLES///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    //Holds the user interface.
    Ui::MainWindow *ui;

    //Holds DataHolder object.
    QList<DataHolder*> data;

    //Holds the signal index of the first plot.
    int plot1Index;

    //Holds the number of the dataholder for the first plot.
    int plot1Data;

    //Holds the signal index of the second plot.
    int plot2Index;

    //Holds the number of the dataholder for the second plot.
    int plot2Data;

    //Holds the signal index of the third plot.
    int plot3Index;

    //Holds the number of the dataholder for the third plot.
    int plot3Data;

    //Holds the list of signals to operate on.
    QList<QListWidgetItem*> signalList;

    //Holds the list of operations to complete.
    QList<QListWidgetItem*> operationList;

    //A temporary place to store activation signals.
    QList<bool> temp;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////FUNCTIONS///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    //Runs operation selection window on arbitrary plot.
    void operationSelector(int plotNum, const QRectF& pos);

    //Prepares a plot, given a reference to the plot, and which area selected
    //signal it is to reference.
    void prepPlot(QwtPlot* plot, int signalNum);

    //Prints curves on a plot.
    void prepCurves(QwtPlot* plot, int signalNum, int dataNum);

    //Loads all points from data. VERY SLOW
    QPolygonF loadPrecisePoints(int col, int dataNum);

    //Loads points from data. Fast, with no visual difference for data with
    //many points.
    QPolygonF loadApproximatePoints(int col, int dataNum);

    //Sets up plot selector boxes.
    void setupSignalBoxes();

    //Sets up single trace list.
    void setupTraceList(QListWidget *list, int signalNum, int dataNum);

    //Sets up all trace lists.
    void setupAllTraceLists();

    //Refreshes curves on all charts.
    void refreshCurves();

    //Removes curves from all plots.
    void clearPlots();

    //Clears data from signal boxes.
    void clearSignalBoxes();

    //Finds the data row for a given time.
    int findRow(float time, int dataNum);

    //Finds the minimum along a single curve between two given times.
    float findMin(float startTime,
                  float endTime,
                  int curveNum,
                  int dataNum);

    //Finds the minimum along a single curve between two given rows.
    float findMin(int startRow,
                  int endRow,
                  int curveNum,
                  int dataNum);

    //Finds the maximum along a single curve between two given rows.
    float findMax(int startRow,
                  int endRow,
                  int curveNum,
                  int dataNum);

    //Finds the minimum for each curve on a plot between two given times.
    QList<float> findMinimums(float startTime,
                              float endTime,
                              int signalNum,
                              int dataNum);

    //Alternate function to find minimums for each curve, between two given
    //rows of data instead of times.
    QList<float> findMinimums(int startRow,
                              int endRow,
                              int signalNum,
                              int dataNum);

    //Averages a single signal.
    void average(int signalNum, int dataNum);

    //Averages all the signals received.
    void average(QList<int> signalNums, int dataNum);

    //Resets the window.
    void reset();

    //Saves data to file.
    void saveData(QString fileUrl);

    //Exports data to a simulation file.
    void exporter(int dataNum,
                  QList<int> signalIndexes,
                  int startRow,
                  int endRow);

    //Reads data from start time to end time. (OBSOLETE)
    QList<QList<float>> readData(float time1, float time2, int dataNum);

    //Opens the manual voltage getter dialog.
    QList<float> manualVoltageGetter(int signalNum,
                                     int dataNum,
                                     int startRow,
                                     int endRow);

    //Runs all voltage getter methods.
    QList<float> voltageGetter(int signalNum,
                               int startRow,
                               int endRow);

    //Convenience function to compare floats.
    bool compare(float num1, float num2, float step);

    //Alternate convenience function to compare floats.
    bool compare(float num1, float num2, int tempStep);

    //Convenience function to fix times.
    void fixTimes(float & startTime, float & endTime, int dataNum);

    //Convenience function to fix a single time.
    float fixTime(float time, int dataNum);

    //Convenience function for using multiple DataHolders.
    void indexFix(int index, int &fixedIndex, int &fixedDataNum);

    //Convenience function to get data from time.
    float getDataFromTime(int dataNum,
                          int signalNum,
                          int traceNum,
                          float time);

    //First step of getting voltage.
    float initialVoltageGetter(int dataNum,
                               QList<int> signalIndexes,
                               int startRow,
                               int endRow,
                               QString title,
                               QString helpText);

    //Writes modeling file out.
    void saveForModeling(QString urlName,
                          QString name,
                          int variant,
                          float voltage,
                          QList<QString *> stepCol1,
                          QList<float *> stepCol2,
                          QList<int *> stepCol3,
                          QList<float> valCol1,
                          QList<float> valCol2,
                          bool longVals = false);

    //Creates the steps in the modeling files.
    bool createStep(int dataNum,
                    QList<int> signalIndexes,
                    QList<QString *>* col1Ref,
                    QList<float *> *col2Ref,
                    QList<int *>* col3Ref,
                    int activationType,
                    bool fluorescence,
                    int startRow,
                    int endRow,
                    QList<float> voltages, int *newSignalNum);

    //Normalizes peaks for Activation and Inactivation traces.
    //This takes in the peak voltage for each trace, and normalizes them
    //by setting the highest peak to 1, lowest to 0, and the others to
    //fractions between them.
    QList<float> normalizePeaks(QList<float> peaks);

    //Normalizes peaks for Trace traces.
    //This takes in the times to find the voltage, the signal being traced,
    //the curve being operated on, and the dataHolder being operated on. It
    //normalizes the curves by finding the lowest and highest values the curve
    //reaches, and setting the lowest to 0, the highest to 1, and then finds
    //the fraction each time step is between the two.
    QList<float> normalizePeaks(QList<float> stepTimes,
                                int signalNum,
                                int curveNum,
                                int dataNum);

private slots:
    //Acts if "Open" is selected in menu. Opens file dialog.
    void on_actionOpen_triggered();

    //Acts if "Exit" is selected in menu. Exits application.
    void on_actionExit_triggered();

    //Acts if plot selector for top plot is changed.
    //Replots curves.
    void on_signalBox1_currentIndexChanged(int index);

    //Acts if plot selector for middle plot is changed.
    //Replots curves.
    void on_signalBox2_currentIndexChanged(int index);

    //Acts if plot selector for bottom plot is changed.
    //Replots curves.
    void on_signalBox3_currentIndexChanged(int index);

    //Acts if an area is selected on top plot.
    void onAreaSelected1(const QRectF & pos);

    //Acts if an area is selected on middle plot.
    void onAreaSelected2(const QRectF & pos);

    //Acts if an area is selected on bottom plot.
    void onAreaSelected3(const QRectF & pos);

    //Acts when given the lists from the signals/operations selector.
    void onListIndexesRecieved(QList<QListWidgetItem*> tempSignalList,
                               QList<QListWidgetItem*> tempOperationList,
                               QList<bool> tempTracesSelected);

    //Acts when "Open files to combine" is selected.
    void on_actionOpen_Files_to_Combine_triggered();

    //Acts when "Save files" is selected.
    void on_actionSave_Data_triggered();

    //Acts when an item on the first trace list is changed.
    void on_traceList1_itemChanged(QListWidgetItem *item);

    //Acts when an item on the second trace list is changed.
    void on_traceList2_itemChanged(QListWidgetItem *item);

    //Acts when an item on the third trace list is changed.
    void on_traceList3_itemChanged(QListWidgetItem *item);

};

#endif // MAINWINDOW_H
