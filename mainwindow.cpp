#include <QFileDialog>
#include <QMessageBox>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>


#include "mainwindow.h"
#include "ui_mainwindow.h"

//Constructor
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    showMaximized();
    setWindowTitle("Modeling Exporter");

    prepPlot(ui->plot1, 1);
    prepPlot(ui->plot2, 2);
    prepPlot(ui->plot3, 3);

    reset();

}

//Destructor
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::operationSelector(int plotNum, const QRectF &pos)
{
    int plotData;
    int plotIndex;
    if (plotNum == 1) {
        plotData = plot1Data;
        plotIndex = plot1Index;
    } else if (plotNum == 2) {
        plotData = plot2Data;
        plotIndex = plot2Index;
    } else if (plotNum == 3) {
        plotData = plot3Data;
        plotIndex = plot3Index;
    }
    if (!data.isEmpty()) {
        float left = pos.left(), right = pos.right();
        fixTimes(left, right, plotData);
        int leftRow = findRow(left, plotData);
        int rightRow = findRow(right, plotData);
        Dialog dialog(*data[plotData], plotIndex, leftRow, rightRow);
        dialog.addSignals(data[plotData]->signalHeader());
        connect(&dialog,
                SIGNAL(listIndexes(QList<QListWidgetItem*>,
                                   QList<QListWidgetItem*>,
                                   QList<bool>)),
                this,
                SLOT(onListIndexesRecieved(QList<QListWidgetItem*>,
                                           QList<QListWidgetItem*>,
                                           QList<bool>)));
        if (dialog.exec()) {

            //Gets signal indexes and calls appropriate operations.
            QList<int> signalIndexes;
            QList<bool> realTracesHolder = data[plotData]->tracesActive();
            data[plotData]->setTracesActive(temp);
            for (int i = 0; i < signalList.length(); i++) {

                QString temp = signalList[i]->text();
                for (int i = 0; i < data[plotData]->signalHeader().length();
                     i++) {
                    if (temp == data[plotData]->signalHeader()[i]) {
                        signalIndexes.append(i);
                    }
                }
            }
            for (int i = 0; i < operationList.length(); i++) {
                if (operationList[i]->text() == "Find peaks.") {
                    for (int i = 0; i < signalIndexes.length(); i++) {
                        QList<float> peaks = findMinimums(left,
                                                 right,
                                                 signalIndexes[i],
                                                 plotData);
                        QString peaksString, title;

                        if (!peaks.isEmpty()) {
                            title = "Peaks Found";
                            peaksString = peaksString.append(QString::number(
                                                                 peaks[0]));
                            for (int i = 1; i < peaks.length(); i++) {
                                peaksString.append(", " + QString::number(
                                                       peaks[i]));
                            }
                        } else {
                            title = "Peaks Not Found";
                        }

                        QMessageBox report(QMessageBox::Information,
                                           title,
                                           peaksString,
                                           QMessageBox::Ok,
                                           this);
                        report.exec();


                    }
                }
                if (operationList[i]->text() == "Find average.") {
                    average(signalIndexes, plotData);
                }
                if (operationList[i]->text() == "Find voltages.") {
                    for (int i = 0; i < signalIndexes.length(); i++) {
                        QList<float> voltages = voltageGetter(signalIndexes[i],
                                                              leftRow,
                                                              rightRow);
                        QString voltagesString, title;

                        if (!voltages.isEmpty()) {
                            title = "Voltages Found";
                            voltagesString = voltagesString.append(
                                        QString::number(voltages[0]));
                            for (int i = 1; i < voltages.length(); i++) {
                                voltagesString.append(
                                            ", " + QString::
                                            number(voltages[i]));
                            }
                        } else {
                            title = "Voltages Not Found";
                        }

                        QMessageBox report(QMessageBox::Information,
                                           title,
                                           voltagesString,
                                           QMessageBox::Ok,
                                           this);
                        report.exec();
                    }
                }
                if (operationList[i]->text() == "Prepare for export.") {
                    exporter(plotData, signalIndexes, leftRow, rightRow);
                }

            }
            data[plotData]->setTracesActive(realTracesHolder);
        }
    }
}


//Prepares a plot, given a reference to the plot, and which area selected
//signal it is to reference.
void MainWindow::prepPlot(QwtPlot* plot, int signalNum)
{
    //Makes background gray.
    plot->setCanvasBackground(Qt::lightGray);

    //Adds ability to select area.
    QwtPlotPicker* selector = new QwtPlotPicker(plot->canvas());
    selector->setStateMachine(new QwtPickerClickRectMachine());
    selector->setRubberBandPen(QColor(Qt::red));
    selector->setRubberBand(QwtPicker::RectRubberBand);
    selector->setTrackerPen(QColor(Qt::red));

    //Connects to signal based on signalNum.
    if (signalNum == 1) {
        connect(selector,
                static_cast<void(QwtPlotPicker::*)(const QRectF &)>
                (&QwtPlotPicker::selected),
                this, &MainWindow::onAreaSelected1);
    } else if (signalNum == 2) {
        connect(selector,
                static_cast<void(QwtPlotPicker::*)(const QRectF &)>
                (&QwtPlotPicker::selected),
                this, &MainWindow::onAreaSelected2);
    } else if (signalNum == 3) {
        connect(selector,
                static_cast<void(QwtPlotPicker::*)(const QRectF &)>
                (&QwtPlotPicker::selected),
                this, &MainWindow::onAreaSelected3);
    } else {
        qDebug() << "ERROR AT SIGNAL ASSIGNMENT";
    }

    //Sets axes to auto-scale to data.
    plot->setAxisAutoScale(0, true);
    plot->setAxisAutoScale(1, true);

    //Shows plot with changes.
    plot->replot();
}

//Prints curves on a plot.
void MainWindow::prepCurves(QwtPlot* plot, int signalNum, int dataNum)
{
    plot->detachItems();
    //Creates list to hold curves.
    QList<QwtPlotCurve*> curveList;

    //Sets up curves, and stores points from DataHolder.
    int counter = 0;

    //Colors inactive traces black.
    for (int i = 1; i < data[dataNum]->signal().length(); i++) {
        if (data[dataNum]->signal()[i].contains(
                    data[dataNum]->signalHeader()[signalNum])
                && !data[dataNum]->tracesActive()[i]) {
            curveList.append(new QwtPlotCurve());
            curveList[counter]->setTitle("");
            curveList[counter]->setPen(Qt::black, 1);
            curveList[counter]->setRenderHint(QwtPlotItem::RenderAntialiased,
                                              true);

            QPolygonF points;

            //Loads all points if few enough. Loads some otherwise.
            if (data[dataNum]->times()[0].length() > 1000) {
                points = loadApproximatePoints(i, dataNum);
            } else {
                points = loadPrecisePoints(i, dataNum);
            }

            //Attaches curve.
            curveList[counter]->setSamples(points);
            curveList[counter]->attach(plot);

            counter++;
        }
    }

    //Colors active traces red.
    for (int i = 1; i < data[dataNum]->signal().length(); i++) {
        if (data[dataNum]->signal()[i].contains(
                    data[dataNum]->signalHeader()[signalNum])
                && data[dataNum]->tracesActive()[i]) {
            curveList.append(new QwtPlotCurve());
            curveList[counter]->setTitle("");
            curveList[counter]->setPen(Qt::red, 2);
            curveList[counter]->setRenderHint(QwtPlotItem::RenderAntialiased,
                                              true);

            QPolygonF points;

            //Loads all points if few enough. Loads some otherwise.
            if (data[dataNum]->times()[0].length() > 1000) {
                points = loadApproximatePoints(i, dataNum);
            } else {
                points = loadPrecisePoints(i, dataNum);
            }

            //Attaches curve.
            curveList[counter]->setSamples(points);
            curveList[counter]->attach(plot);

            counter++;
        }
    }

    //Adds grid.
    QwtPlotGrid* grid1 = new QwtPlotGrid();
    grid1->attach(plot);

    //Shows updated plot.
    plot->replot();
}

//Loads all points from data. VERY SLOW
QPolygonF MainWindow::loadPrecisePoints(int col, int dataNum)
{
    QPolygonF points;
    for (int i = 0; i < data[dataNum]->times()[0].length() - 1; i++) {
        points << QPointF(data[dataNum]->times()[0][i],
                data[dataNum]->times()[col][i]);
    }
    return points;
}

//Loads points from data. Fast, with no visual difference for data with
//many points.
QPolygonF MainWindow::loadApproximatePoints(int col, int dataNum)
{
    QPolygonF points;
    for (int i = 0; i < data[dataNum]->times()[0].length() - 1;
         i = i + (data[dataNum]->times()[0].length())/1000) {
        points << QPointF(data[dataNum]->times()[0][i],
                data[dataNum]->times()[col][i]);
    }
    return points;
}

//Sets up plot selector boxes.
void MainWindow::setupSignalBoxes()
{
    int seperatorCounter = 0;
    //Adds signals.
    for (int i = 0; i < data.length(); i++) {
        for (int j = 0; j < data[i]->signalHeader().length(); j++) {
            ui->signalBox1->addItem(data[i]->signalHeader()[j]);
            ui->signalBox2->addItem(data[i]->signalHeader()[j]);
            ui->signalBox3->addItem(data[i]->signalHeader()[j]);
        }
    }
    //Adds dividers if necessary to denote different files.
    for (int i = 0; i < data.length()-1; i++) {
        for (int j = 0; j < data[i]->signalHeader().length(); j++) {
            seperatorCounter++;
        }
        ui->signalBox1->insertSeparator(seperatorCounter);
        ui->signalBox2->insertSeparator(seperatorCounter);
        ui->signalBox3->insertSeparator(seperatorCounter);
        seperatorCounter++;
    }
}

//Sets up trace lists.
void MainWindow::setupTraceList(QListWidget* list,int signalNum, int dataNum)
{
    list->clear();
    //Finds the traces to display.
    for (int i = 1; i < data[dataNum]->traces().length(); i++) {
        if (data[dataNum]->signal()[i].contains(
                    data[dataNum]->signalHeader()[signalNum])) {
            //Creates new list item.
            QListWidgetItem* temp
                    = new QListWidgetItem(data[dataNum]->traces()[i]);

            //Makes the list item checkable.
            temp->setFlags(temp->flags() | Qt::ItemIsUserCheckable);

            //Sets check to see whether the trace is active.
            if (data[dataNum]->tracesActive()[i]) {
                temp->setCheckState(Qt::Checked);
            } else {
                temp->setCheckState(Qt::Unchecked);
            }
            //Adds the data, then the item to the list to display.
            temp->setData(Qt::UserRole, QVariant(i));
            list->addItem(temp);
        }
    }
}

//Sets up all trace lists.
void MainWindow::setupAllTraceLists()
{
    setupTraceList(ui->traceList1, plot1Index, plot1Data);
    setupTraceList(ui->traceList2, plot2Index, plot2Data);
    setupTraceList(ui->traceList3, plot3Index, plot3Data);
}

//Refreshes curves on all charts.
void MainWindow::refreshCurves()
{
    prepCurves(ui->plot1, plot1Index, plot1Data);
    prepCurves(ui->plot2, plot2Index, plot2Data);
    prepCurves(ui->plot3, plot3Index, plot3Data);
}

//Removes curves from all plots.
void MainWindow::clearPlots()
{
    ui->plot1->detachItems();
    ui->plot2->detachItems();
    ui->plot3->detachItems();
}

//Clears data from signal boxes.
void MainWindow::clearSignalBoxes() {
    ui->signalBox1->clear();
    ui->signalBox2->clear();
    ui->signalBox3->clear();
}


//Finds the data row for a given time.
int MainWindow::findRow(float time, int dataNum)
{
    int startCol = 0;
    int step = ((data[dataNum]->times()[0][1]
            - data[dataNum]->times()[0][0])*100000);
    while (!compare(time, data[dataNum]->times()[0][startCol], step)) {
        startCol++;
    }
    return startCol;
}

//Finds the minimum along a single curve between two given times.
float MainWindow::findMin(float startTime,
                          float endTime,
                          int curveNum,
                          int dataNum)
{
    int startCol = findRow(startTime, dataNum), endCol
            = findRow(endTime, dataNum);

    float minimum = data[dataNum]->times()[curveNum][startCol];
    for (int i = startCol; i < endCol; i++) {
        if (data[dataNum]->times()[curveNum][i] < minimum) {
            minimum = data[dataNum]->times()[curveNum][i];
        }
    }


    return minimum;
}

//Finds the minimum along a single curve between two given rows.
float MainWindow::findMin(int startRow, int endRow, int curveNum, int dataNum)
{

    float minimum = data[dataNum]->times()[curveNum][startRow];
    for (int i = startRow; i < endRow; i++) {
        if (data[dataNum]->times()[curveNum][i] < minimum) {
            minimum = data[dataNum]->times()[curveNum][i];
        }
    }


    return minimum;
}

//Finds the maximum along a single curve between two given rows.
float MainWindow::findMax(int startRow, int endRow, int curveNum, int dataNum)
{

    float maximum = data[dataNum]->times()[curveNum][startRow];
    for (int i = startRow; i < endRow; i++) {
        if (data[dataNum]->times()[curveNum][i] > maximum) {
            maximum = data[dataNum]->times()[curveNum][i];
        }
    }

    return maximum;
}

//Finds the minimum for each curve on a plot between two given times.
//
//PLEASE NOTE: This function will return a value for every trace, regardless of
//whether that trace is active. However, if a trace is inactive, it will return
//"0" instead of the actual minimum. This was done to ensure the values would
//fit consistantly into data tables.
QList<float> MainWindow::findMinimums(float startTime,
                                      float endTime,
                                      int signalNum,
                                      int dataNum)
{
    QList<float> minimums;

    for (int i = 1; i < data[dataNum]->signal().length(); i++) {
        if (data[dataNum]->signal()[i].contains(
                    data[dataNum]->signalHeader()[signalNum])) {
            if (data[dataNum]->tracesActive()[i]) {
                minimums.append(findMin(startTime,
                                        endTime,
                                        i,
                                        dataNum));
            } else {
                minimums.append(NULL);
            }
        }
    }
    return minimums;
}

//Alternate function to find minimums for each curve, between two given
//rows of data instead of times.
QList<float> MainWindow::findMinimums(int startRow,
                                      int endRow,
                                      int signalNum,
                                      int dataNum)
{
    QList<float> minimums;

    for (int i = 1; i < data[dataNum]->signal().length(); i++) {
        if (data[dataNum]->signal()[i].contains(
                    data[dataNum]->signalHeader()[signalNum])) {
            if (data[dataNum]->tracesActive()[i]) {
                minimums.append(findMin(startRow,
                                        endRow,
                                        i,
                                        dataNum));
            } else {
                minimums.append(NULL);
            }
        }
    }
    return minimums;
}

//Averages a single signal.
void MainWindow::average(int signalNum, int dataNum)
{
    QList<int> temp;
    temp.append(signalNum);
    average(temp, dataNum);
}

//Averages all the signals received.
void MainWindow::average(QList<int> signalNums, int dataNum)
{
    DataHolder* temp = new DataHolder(*data[dataNum]);
    for (int i = 0; i < signalNums.length(); i++) {
        temp->average(signalNums[i]);
    }
    data.append(temp);
    clearSignalBoxes();
    setupSignalBoxes();
}

//Resets the window.
void MainWindow::reset()
{
    clearSignalBoxes();
    clearPlots();
    data.clear();
    plot1Index = plot2Index = plot3Index = -1;
    plot1Data = plot2Data = plot3Data = -1;
}

//Saves data to file.
void MainWindow::saveData(QString fileUrl)
{
    if (plot1Data != -1) {
        data[plot1Data]->saveData(fileUrl);
    }
}

//Exports data to a simulation file.
void MainWindow::exporter(int dataNum,
                          QList<int> signalIndexes,
                          int startRow,
                          int endRow)
{
    //Sets up message boxes.
    QMessageBox exitWarning(QMessageBox::Warning,
                            "Continue Exporting?",
                            "Would you like to continue exporting?",
                            QMessageBox::Yes
                            |QMessageBox::Reset
                            |QMessageBox::No,
                            this,
                            Qt::Dialog);

    float voltage = initialVoltageGetter(dataNum,
                                         signalIndexes,
                                         startRow,
                                         endRow,
                                         "Please select a voltage...",
                                         "Using the output voltage graph, "
                                         "please select a voltage to be used"
                                         " for the "
                                         "initial voltage level.");

    //Getting initial voltage {
    while (voltage == NULL) {
        int exiter = exitWarning.exec();
        if (exiter == QMessageBox::No) {
            return;
        } else if (exiter == QMessageBox::Reset) {
            exporter(dataNum, signalIndexes, startRow, endRow);
            return;
        } else {
            voltage = initialVoltageGetter(dataNum,
                                           signalIndexes,
                                           startRow,
                                           endRow,
                                           "Please select a voltage...",
                                           "Using the output voltage graph, "
                                           "please select a voltage to be used"
                                           " for the "
                                           "initial voltage level.");
        }
    }
    QMessageBox check(QMessageBox::Question,
                      "Voltage OK?",
                      "Is this the initial voltage you want to use?"
                      "\n"+QString::number((int)(voltage))+
                      "\nYou will have a chance to adjust this number.",
                      QMessageBox::Yes
                      |QMessageBox::No,
                      this,
                      Qt::Dialog);
    while (check.exec() != QMessageBox::Yes) {
        voltage = initialVoltageGetter(dataNum,
                                       signalIndexes,
                                       startRow,
                                       endRow,
                                       "Please select a voltage...",
                                       "Using the output voltage graph, "
                                       "please select a voltage to be used for"
                                       " the "
                                       "initial voltage level.");
        check.setText("Is this the initial voltage you want to use?"
                      "\n"+QString::number((int)(voltage))+
                      "\nYou will have a chance to adjust this number.");
        while (voltage == NULL) {
            int exiter = exitWarning.exec();
            if (exiter == QMessageBox::No) {
                return;
            } else if (exiter == QMessageBox::Reset) {
                exporter(dataNum,
                         signalIndexes,
                         startRow,
                         endRow);
                return;
            } else {
                voltage = initialVoltageGetter(dataNum,
                                               signalIndexes,
                                               startRow,
                                               endRow,
                                               "Please select a voltage...",
                                               "Using the output voltage graph, "
                                               "please select a voltage to be"
                                               " used for the "
                                               "initial voltage level.");
            }
        }
    }
    // } End getting initial voltage.

    //Run export for to get export basics.
    int* fixedVoltage = new int(voltage);
    int* indexReference = new int(-1);
    QString* title = new QString();
    ExportForm exportForm(fixedVoltage, indexReference, title);
    while (exportForm.exec() != QDialog::Accepted) {
        int exiter = exitWarning.exec();
        if (exiter == QMessageBox::No) {
            return;
        } else if (exiter == QMessageBox::Reset) {
            exporter(dataNum, signalIndexes, startRow, endRow);
            return;
        }
    }



    //Runs window to get data about step type.
    QList<QString*> col1Ref;
    QList<float*> col2Ref;
    QList<int*> col3Ref;
    int* col3 = new int;
    QInputDialog newStep;
    bool ok = false;
    QStringList items;
    items << "Activation" << "Inactivation" << "Trace";


    QString response = newStep.getItem(this,
                                       "Select Step Type",
                                       "Please select"
                                       " the type of step you would"
                                       " like to add",
                                       items,
                                       0,
                                       false,
                                       &ok);

    while (!ok ||response.isEmpty()) {
        int exiter = exitWarning.exec();
        if (exiter == QMessageBox::No) {
            return;
        } else if (exiter == QMessageBox::Reset) {
            exporter(dataNum, signalIndexes, startRow, endRow);
            return;
        } else {
            response = newStep.getItem(this,
                                       "Select Step Type",
                                       "Please select the type of step you"
                                       " would like to add",
                                       items,
                                       0,
                                       false,
                                       &ok);
        }
    }


    //Analyzes response.
    if (response == items[0]) {
        *col3 = 0;
    } else if (response == items[1]) {
        *col3 = 1;
    } else if (response == items[2]) {
        *col3 = 2;
    }

    //Runs window to check if data is fluorescence data.
    QMessageBox fluorescenceChecker(QMessageBox::NoIcon,
                                    "Fluorescence?",
                                    "Is this fluorescence data?",
                                    QMessageBox::Yes|
                                    QMessageBox::No|
                                    QMessageBox::Cancel);
    int checker = fluorescenceChecker.exec();
    while (checker == QMessageBox::Cancel) {
        int exiter = exitWarning.exec();
        if (exiter == QMessageBox::No) {
            return;
        } else if (exiter == QMessageBox::Reset) {
            exporter(dataNum, signalIndexes, startRow, endRow);
            return;
        } else {
            checker = fluorescenceChecker.exec();
        }
    }

    //Gets voltages for traces.
    QList<float> voltages = voltageGetter(signalIndexes[0], startRow, endRow);
    while (voltages.isEmpty()) {
        int exiter = exitWarning.exec();
        if (exiter == QMessageBox::No) {
            return;
        } else if (exiter == QMessageBox::Reset) {
            exporter(dataNum, signalIndexes, startRow, endRow);
            return;
        } else {
            voltages = voltageGetter(signalIndexes[0], startRow, endRow);
        }
    }

    //Begins creation of individual steps.
    int* newSignalNum = new int;

    while (!createStep(dataNum,
                       signalIndexes,
                       &col1Ref,
                       &col2Ref,
                       &col3Ref,
                       *col3,
                       checker == QMessageBox::Yes,
                       startRow,
                       endRow,
                       voltages,
                       newSignalNum)) {
        int exiter = exitWarning.exec();
        if (exiter == QMessageBox::No) {
            return;
        } else if (exiter == QMessageBox::Reset) {
            exporter(dataNum, signalIndexes, startRow, endRow);
            return;
        }
    }

    QList<float> valsCol1, valsCol2;

    //Runs for trace vs. not trace protocols.
    if (*col3 != 2) {
        //Non-trace protocols. Normalizes peaks normally.
        valsCol1 = voltages;
        valsCol2 = normalizePeaks(findMinimums(startRow,
                                               endRow,
                                               *newSignalNum,
                                               dataNum));
    } else {
        //Trace protocols. Uses alternate nomalization.

        //Runs two windows to determine how long to read trace, and how many
        //steps to use (currently, the steps are set to a maximum of 40,
        //with 20 being the recommended amount.
        float step = (data[dataNum]->times()[0][1] -
                data[dataNum]->times()[0][0])*10000;
        float timeLength = (float)(newStep.getDouble(this,
                                                     "Choose time duration...",
                                                     "Please select how long "
                                                     "you wish "
                                                     "to track this trace.",
                                                     step*20,
                                                     0.0,
                                                     step*(
                                                         data[dataNum]->times()
                                                         [0].length()),
                                   4,
                                   &ok));
        while (!ok) {
            int exiter = exitWarning.exec();
            if (exiter == QMessageBox::No) {
                return;
            } else if (exiter == QMessageBox::Reset) {
                exporter(dataNum, signalIndexes, startRow, endRow);
                return;
            } else {
                timeLength = (float)(newStep.getDouble(this,
                                                     "Choose time duration...",
                                                     "Please select how long"
                                                     " you wish "
                                                     "to track this trace.",
                                                     step*20,
                                                     0.0,
                                                     step*(data[dataNum]
                                                           ->times()
                                                           [0].length()),
                                     4,
                                     &ok));
            }
        }
        int timeSteps = newStep.getInt(this,
                                       "Choose number of steps...",
                                       "Please choose how many steps to split"
                                       " the time to track into.",
                                       20,
                                       0,
                                       40,
                                       1,
                                       &ok);

        while (!ok) {
            int exiter = exitWarning.exec();
            if (exiter == QMessageBox::No) {
                return;
            } else if (exiter == QMessageBox::Reset) {
                exporter(dataNum, signalIndexes, startRow, endRow);
                return;
            } else {
                timeSteps = newStep.getInt(this,
                                           "Choose number of steps...",
                                           "Please choose how many steps "
                                           "to split"
                                           " the time to track into.",
                                           20,
                                           0,
                                           40,
                                           1,
                                           &ok);
            }
        }

        //Fixes time steps and normalizes peaks.
        float time = 0;
        float dividedStep = (timeLength/timeSteps);
        for (int i = 0; i < timeSteps; i++) {
            valsCol1.append(time);
            time = time + dividedStep;
        }
        valsCol2 = normalizePeaks(valsCol1,
                                  *newSignalNum,
                                  *indexReference,
                                  dataNum);
    }



    //Gets place to save file.
    QFileDialog saver;
    QString urlName = saver.getSaveFileName(this,
                                            "Export to...",
                                            *title,
                                            "No Extension ()");
    while (urlName.isEmpty()) {
        int exiter = exitWarning.exec();
        if (exiter == QMessageBox::No) {
            return;
        } else if (exiter == QMessageBox::Reset) {
            exporter(dataNum, signalIndexes, startRow, endRow);
            return;
        } else {
            urlName = saver.getSaveFileName(this,
                                            "Export to...",
                                            *title,
                                            "No Extension ()");
        }
    }


    //Saves file.
    saveForModeling(urlName,
                     *title,
                     *indexReference,
                     *fixedVoltage,
                     col1Ref,
                     col2Ref,
                     col3Ref,
                     valsCol1,
                     valsCol2,
                     *col3 == 2);
}

//Creates the steps in the modeling files.
bool MainWindow::createStep(int dataNum,
                            QList<int> signalIndexes,
                            QList<QString*>* col1Ref,
                            QList<float*>* col2Ref,
                            QList<int*>* col3Ref,
                            int activationType,
                            bool fluorescence,
                            int startRow,
                            int endRow,
                            QList<float> voltages,
                            int* newSignalNum)
{
    for (int i = 0; i < signalIndexes.length(); i++) {
        if (activationType == 0) {
            //Activation
            col1Ref->append(new QString(EXPORT_VARIABLE));
            float* startTime = new float(0.0);
            float* endTime = new float(0.0);
            VoltageDialog timeGetter(*data[dataNum],
                                     "Please select times...",
                                     "Using the channel voltage graph, "
                                     "please select a start time and"
                                     " a stop time "
                                     "that include the peaks of all traces.",
                                     startTime,
                                     endTime,
                                     signalIndexes[i],
                                     startRow,
                                     endRow,
                                     newSignalNum);
            if (timeGetter.exec() == QDialog::Rejected) {
                return false;
            }
            col2Ref->append(new float(fixTime(
                                          (*endTime-*startTime), dataNum)));
            if (fluorescence) {
                col3Ref->append(new int(3));
            } else {
                col3Ref->append(new int(1));
            }
        } else if (activationType == 1) {
            //Inactivation

            //Step 1
            col1Ref->append(new QString(EXPORT_VARIABLE));
            col2Ref->append(new float(200.0));
            col3Ref->append(new int(0));

            //Step 2
            float* flatVoltage = new float;
            VoltageDialog flatVoltageGetter(*data[dataNum],
                                            "Please select first voltage...",
                                            "Using the output voltage graph, "
                                            "please select the voltage of"
                                            " the first step"
                                            " (The step before the trace"
                                            " voltages split).",
                                            flatVoltage,
                                            signalIndexes[i],
                                            startRow,
                                            endRow);
            flatVoltageGetter.exec();
            col1Ref->append(new QString(QString::number(*flatVoltage)));
            float* startTime = new float(0.0);
            float* endTime = new float(0.0);
            VoltageDialog timeGetter(*data[dataNum],
                                     "Please select times...",
                                     "Using the channel voltage graph, "
                                     "please select a start time"
                                     " and a stop time "
                                     "that include the peaks of all traces.",
                                     startTime,
                                     endTime,
                                     signalIndexes[i],
                                     startRow,
                                     endRow,
                                     newSignalNum);
            if (timeGetter.exec() == QDialog::Rejected) {
                return false;
            }
            col2Ref->append(new float(fixTime(
                                          (*endTime-*startTime), dataNum)));
            if (fluorescence) {
                col3Ref->append(new int(3));
            } else {
                col3Ref->append(new int(1));
            }
        } else if (activationType == 2) {
            //Trace
            int* traceNum = new int;
            TracePickDialog traceGetter(*data[dataNum],
                                        signalIndexes[i],
                                        "Select a single trace to operate on.",
                                        startRow,
                                        endRow,
                                        this,
                                        false,
                                        traceNum);
            traceGetter.exec();
            int counter = 0;
            for (int j = 1; j < data[dataNum]->traces().length(); j++) {
                if (data[dataNum]->signal()[j].contains(
                            data[dataNum]->signalHeader()[signalIndexes[i]])) {
                    if (j == *traceNum) {
                        col1Ref->append(new QString(
                                            QString::number(
                                                voltages[counter])));
                    }
                    counter++;
                }
            }
            float* startTime = new float(0.0);
            float* endTime = new float(0.0);
            VoltageDialog timeGetter(*data[dataNum],
                                     "Please select times...",
                                     "Using the channel voltage graph, "
                                     "please select a start time"
                                     " and a stop time "
                                     "that include the peaks of all traces.",
                                     startTime,
                                     endTime,
                                     signalIndexes[i],
                                     startRow,
                                     endRow,
                                     newSignalNum);
            if (timeGetter.exec() == QDialog::Rejected) {
                return false;
            }
            col2Ref->append(new float(fixTime(
                                          (*endTime-*startTime), dataNum)));
            if (fluorescence) {
                col3Ref->append(new int(4));
            } else {
                col3Ref->append(new int(2));
            }
        } else {
            qDebug() << "Error in activation type selection";
        }
    }
    return true;
}

//Normalizes peaks for Activation and Inactivation traces.
//This takes in the peak voltage for each trace, and normalizes them
//by setting the highest peak to 1, lowest to 0, and the others to
//fractions between them.
QList<float> MainWindow::normalizePeaks(QList<float> peaks)
{
    float lowest, highest;
    lowest = highest = peaks[0];
    for (int i = 0; i < peaks.length(); i++) {
        if (peaks[i] < lowest) {
            lowest = peaks[i];
        }
        if (peaks[i] > highest) {
            highest = peaks[i];
        }
    }
    for (int i = 0; i < peaks.length(); i++) {
        peaks[i] = peaks[i] - highest;
    }
    for (int i = 0; i < peaks.length(); i++) {
        peaks[i] = (peaks[i]/(lowest - highest));
    }

    return peaks;
}

//Normalizes peaks for Trace traces.
//This takes in the times to find the voltage, the signal being traced,
//the curve being operated on, and the dataHolder being operated on. It
//normalizes the curves by finding the lowest and highest values the curve
//reaches, and setting the lowest to 0, the highest to 1, and then finds
//the fraction each time step is between the two.
QList<float> MainWindow::normalizePeaks(QList<float> stepTimes,
                                        int signalNum,
                                        int curveNum,
                                        int dataNum)
{
    float lowest = 0, highest = 0;
    int counter = 0;
    for (int i = 1; i < data[dataNum]->signal().length(); i++) {
        if (data[dataNum]->signal()[i].contains(
                    data[dataNum]->signalHeader()[signalNum])) {
            if (counter == curveNum) {
                lowest = findMin(findRow(data[dataNum]
                                         ->times()[0][0], dataNum),
                        findRow(data[dataNum]->times()[0]
                        [data[dataNum]->times()[0].length()-1],
                        dataNum),
                        i,
                        dataNum);
                highest = findMax(findRow(data[dataNum
                                          ]->times()[0][0], dataNum),
                        findRow(data[dataNum]->times()[0]
                        [data[dataNum]->times()[0].length()-1],
                        dataNum),
                        i,
                        dataNum);
                counter++;
            }
        }
    }

    for (int i = 0; i < stepTimes.length(); i++) {
        stepTimes[i] = stepTimes[i]/1000;
        stepTimes[i] = getDataFromTime(dataNum,
                                       signalNum,
                                       curveNum,
                                       stepTimes[i]);
        stepTimes[i] = stepTimes[i] - highest;
    }
    for (int i = 0; i < stepTimes.length(); i++) {
        stepTimes[i] = (stepTimes[i]/(lowest-highest));
    }

    return stepTimes;
}

//Reads data from start time to end time (OBSOLETE)
QList<QList<float>> MainWindow::readData(float time1, float time2, int dataNum)
{
    if (time1 <= time2) {
        int startCol = 0, endCol = 0;
        float step = (data[dataNum]->times()[0][1]
                - data[dataNum]->times()[0][0]);
        while (!compare(time1, data[dataNum]->times()[0][startCol], step)) {
            startCol++;
        }
        endCol = startCol;
        while (!compare(time2, data[dataNum]->times()[0][endCol], step)) {
            endCol++;
        }
        QList<QList<float>> temp;
        for (int i = 0; i < data[dataNum]->traces().length(); i++) {
            int col = startCol;
            if (temp.length() < data[dataNum]->traces().length()) {
                temp.append(*(new QList<float>));
            }
            while (col < endCol) {
                temp[i].append(data[dataNum]->times()[i][col]);
                col++;
            }
        }
        return temp;
    } else {
        qDebug() << "ERROR: INVALID TIMES CALLED FOR DATA READING";
        QList<QList<float>> temp;
        return temp;
    }
}

//Opens the manual voltage getter dialog.
QList<float> MainWindow::manualVoltageGetter(int signalNum,
                                             int dataNum,
                                             int startRow,
                                             int endRow)
{
    QList<float> voltageList;
    QMessageBox cannotBeFound(QMessageBox::Question,
                              "Voltages Aligned?",
                              "Do the voltages being "
                              "selected all occur at the same time?",
                              QMessageBox::Yes,
                              this,
                              Qt::Dialog);
    cannotBeFound.addButton(QMessageBox::No);
    if (cannotBeFound.exec() == QMessageBox::Yes) {
        bool selected = false;
        float* voltageTime = new float(0);
        for (int i = 1; i < data[dataNum]->signal().length(); i++) {
            if (data[dataNum]->signal()[i].contains(
                        data[dataNum]->signalHeader()[signalNum])
                    && data[dataNum]->tracesActive()[i] && !selected) {
                VoltageDialog temp(*data[dataNum],
                                   "Please select a voltage...",
                                   "Using the output voltage graph, "
                                   "please select a voltage for the traces"
                                   " you wish to operate on.",
                                   voltageTime,
                                   signalNum,
                                   i,
                                   startRow,
                                   endRow);
                if (!selected && temp.exec() == QDialog::Rejected) {
                    voltageList.clear();
                    return voltageList;
                }
                *voltageTime = fixTime(*voltageTime, dataNum);
                float voltage
                        = data[dataNum]->times()[i][findRow(*voltageTime,
                                                            dataNum)];
                voltageList.append(voltage);
                selected = true;

            } else if (data[dataNum]->signal()[i].contains(
                           data[dataNum]->signalHeader()[signalNum])
                       && data[dataNum]->tracesActive()[i] && selected) {
                float voltage
                        = data[dataNum]->times()[i][findRow(*voltageTime,
                                                            dataNum)];
                voltageList.append(voltage);
            }
        }
    } else {
        for (int i = 1; i < data[dataNum]->signal().length(); i++) {
            if (data[dataNum]->signal()[i].contains(
                        data[dataNum]->signalHeader()[signalNum])
                    && data[dataNum]->tracesActive()[i]) {
                float* voltage = new float(0);
                VoltageDialog temp(*data[dataNum],
                                   "Please select a voltage...",
                                   "Using the output voltage graph, "
                                   "please select a voltage for the traces"
                                   " you wish to operate on.",
                                   voltage,
                                   signalNum,
                                   i,
                                   startRow,
                                   endRow);
                if (temp.exec() == QDialog::Rejected) {
                    voltageList.clear();
                    return voltageList;
                }
                *voltage = fixTime(*voltage, dataNum);
                *voltage = data[dataNum]->times()[i][findRow(*voltage,
                                                             dataNum)];
                voltageList.append(*voltage);
            }
        }
    }
    return voltageList;
}

//Runs all voltage getter methods.
QList<float> MainWindow::voltageGetter(int signalNum,
                                       int startRow,
                                       int endRow)
{

    QList<float> voltages;
    voltages = data[plot1Data]->findVoltages(signalNum);
    while (voltages.isEmpty()) {
        QMessageBox cannotBeFound(QMessageBox::Question,
                                  "Voltages Could Not "
                                  "Be Found",
                                  "Enter Manual Selector?",
                                  QMessageBox::Yes,
                                  this,
                                  Qt::Dialog);
        cannotBeFound.addButton(QMessageBox::No);
        int cannotBeFoundCheck = cannotBeFound.exec();
        if (cannotBeFoundCheck == QMessageBox::Yes) {
            voltages = manualVoltageGetter(signalNum,
                                           plot1Data,
                                           startRow,
                                           endRow);
        } else if (cannotBeFoundCheck == QMessageBox::No){
            voltages.clear();
            return voltages;
        }
    }
    QMessageBox check(QMessageBox::Question,
                      "Correct Voltages Found?",
                      "Are these the voltages you want"
                      " to use?",
                      QMessageBox::Yes|QMessageBox::No,
                      this,
                      Qt::Dialog);
    QString message = "Are these the voltages you want"
                      " to use?\n";
    message.append((QString::number(voltages[0])));
    for (int j = 1; j < voltages.length(); j++) {
        message.append(", ");
        message.append(QString::number(voltages[j]));
    }
    check.setText(message);
    int checked = check.exec();
    while (checked == QMessageBox::No) {
        QMessageBox cannotBeFound(
                    QMessageBox::Question,
                    "Voltages Could Not Be Found",
                    "Enter Manual Selector?",
                    QMessageBox::Yes|QMessageBox::No,
                    this,
                    Qt::Dialog);
        int cannotBeFoundCheck = cannotBeFound.exec();
        if (cannotBeFoundCheck == QMessageBox::Yes) {
            voltages = manualVoltageGetter(
                        signalNum,
                        plot1Data,
                        startRow,
                        endRow);
            if (!voltages.isEmpty()) {
                message = "Are these the voltages you want"
                          " to use?\n";
                message.append((QString::number(voltages[0])));
                for (int j = 1; j < voltages.length(); j++) {
                    message.append(", ");
                    message.append(QString::number(voltages[j]));
                }
                check.setText(message);
                checked = check.exec();
            } else {
                voltages.clear();
                return voltages;
            }
        } else if (cannotBeFoundCheck
                   == QMessageBox::No) {
            voltages.clear();
            return voltages;
        }
    }
    return voltages;
}


//Convenience function to compare floats.
bool MainWindow::compare(float num1, float num2, float step)
{
    int tempStep = step*100000;
    return compare(num1, num2, tempStep);
}

//Alternate convenience function to compare floats.
bool MainWindow::compare(float num1, float num2, int tempStep) {
    int tempNum1 = num1*100000, tempNum2 = num2*100000;

    if (tempNum1%tempStep != 0) {
        tempNum1 = tempNum1 + (tempStep - (tempNum1%tempStep));
    }
    if (tempNum2%tempStep != 0) {
        tempNum2 = tempNum2 + (tempStep - (tempNum2%tempStep));
    }
    return tempNum1 == tempNum2;
}

//Convenience function to fix times.
void MainWindow::fixTimes(float &startTime, float &endTime, int dataNum)
{
    float minTime = data[dataNum]->times()[0][0];
    float maxTime
            = data[dataNum]->times()[0][data[dataNum]->times()[0].length()-1];
    float step = data[dataNum]->times()[0][1] - data[dataNum]->times()[0][0];
    //Fixes left bound
    if (endTime < minTime) {
        endTime = minTime;
        startTime = minTime;
    } else if (startTime < minTime) {
        startTime = minTime;
    }

    //Fixes right bound
    if (startTime > maxTime) {
        startTime = maxTime;
        endTime = maxTime;
    } else if (endTime > maxTime) {
        endTime = maxTime;
    }

    //Fixes equality
    if (compare(startTime, endTime, step) && compare(endTime,minTime, step)) {
        endTime = data[dataNum]->times()[0][1];
    } else if (compare(startTime, endTime, step)
               && compare(startTime, maxTime, step)) {
        startTime
                = data[dataNum]->times()[0][data[dataNum]->times()[0].length()
                -2];
    } else if (compare(startTime, endTime, step)) {
        endTime = endTime + step;
    }
}

//Convenience function to fix a single time.
float MainWindow::fixTime(float time, int dataNum)
{
    float minTime = data[dataNum]->times()[0][0];
    float maxTime
            = data[dataNum]->times()[0][data[dataNum]->times()[0].length()-1];
    if (time < minTime) {
        time = minTime;
    }
    if (time > maxTime) {
        time = maxTime;
    }
    return time;
}

//Convenience function for using multiple DataHolders.
void MainWindow::indexFix(int index, int &fixedIndex, int &fixedDataNum)
{
    for (int i = 0; i < data.length(); i++) {
        for (int j = 0; j < data[i]->signalHeader().length(); j++) {
            if (index == 0) {
                fixedDataNum = i;
                fixedIndex = j;
                return;
            }
            index--;
        }
        index--;
    }
}

//Convenience function to get data from time.
float MainWindow::getDataFromTime(int dataNum,
                                  int signalNum,
                                  int traceNum,
                                  float time)
{
    int counter = 0;
    float temp = NULL;
    for (int i = 1; i < data[dataNum]->signal().length(); i++) {
        if (data[dataNum]->signal()[i].contains(
                    data[dataNum]->signalHeader()[signalNum])) {

            if (counter == traceNum) {
                temp = data[dataNum]->times()[i][findRow(time, dataNum)];
                return temp;
            }
            counter++;
        }
    }
    return NULL;
}

//First step of getting voltage.
float MainWindow::initialVoltageGetter(int dataNum,
                                       QList<int> signalIndexes,
                                       int startRow,
                                       int endRow,
                                       QString title,
                                       QString helpText)
{
    float* voltage = new float(0);
    for (int j = 0; j < signalIndexes.length(); j++) {
        for (int i = 1; i < data[dataNum]->signal().length(); i++) {
            if (data[dataNum]->signal()[i].contains(
                        data[dataNum]->signalHeader()[signalIndexes[j]])
                    && data[dataNum]->tracesActive()[i]) {
                VoltageDialog initialGetter(*data[dataNum],
                                            title,
                                            helpText,
                                            voltage,
                                            signalIndexes[j],
                                            i,
                                            startRow,
                                            endRow);
                if (initialGetter.exec() == QDialog::Accepted) {
                    return getDataFromTime(dataNum, j, i, *voltage);
                } else {
                    return NULL;
                }
            }
        }
    }
    return NULL;
}

//Writes modeling file out.
void MainWindow::saveForModeling(QString urlName,
                                  QString name,
                                  int variant,
                                  float voltage,
                                  QList<QString *> stepCol1,
                                  QList<float *> stepCol2,
                                  QList<int *> stepCol3,
                                  QList<float> valCol1,
                                  QList<float> valCol2,
                                  bool longVals)
{
    QFile file(urlName);
    if (!file.open(QIODevice::ReadWrite|QIODevice::Truncate)) {
        qDebug() << "ERROR EXPORTING FILE";
        return;
    }
    QTextStream outFile(&file);

    outFile.setRealNumberPrecision(1);
    outFile.setRealNumberNotation(QTextStream::FixedNotation);

    outFile << "name:\t";

    outFile << name;

    outFile << "\nv0:\t" << voltage;

    outFile << "\n\n";

    outFile << "variant:\t";

    outFile << variant;

    outFile << "\n\n";

    outFile << "steps:" << endl;

    for (int i = 0; i < stepCol1.length(); i++) {
        outFile << *stepCol1[i]
                   << "\t\t" << ((*stepCol2[i])*1000)
                   << "\t\t" << *stepCol3[i] << endl;
    }

    outFile << "end";

    outFile << "\n\n";

    outFile << "vals:" << endl;

    for (int i = 0; i < valCol1.length(); i++) {
        if (longVals) {
            outFile.setRealNumberPrecision(4);
        } else {
            outFile.setRealNumberPrecision(1);
        }
        outFile << valCol1[i] << "\t\t";
        outFile.setRealNumberPrecision(4);
        outFile << valCol2[i] << endl;
    }

    outFile << "end";

    file.close();
}



//Acts if "Open" is selected in menu. Opens file dialog.
void MainWindow::on_actionOpen_triggered()
{
    QFileDialog openDialog;

    QStringList fileUrl
            = openDialog.getOpenFileNames(this,
                                          tr("Open File"), "" ,
                                          tr("Text Files (*.txt, *.atf)"));

    if (!fileUrl.isEmpty()) {

        reset();

        for (int i = 0; i < fileUrl.length(); i++) {
            if (data.length() > 0) {
                bool loaded = false;
                for (int j = 0; j < data.length(); j++) {
                    if (fileUrl[i] == NULL
                            || data[j]->fileName() == fileUrl[i]) {
                        loaded = true;
                    }
                }
                if (!loaded) {
                    data.append(new DataHolder);

                    data[data.length()-1]->loadData(fileUrl[i]);

                    loaded = false;
                }
            } else {
                data.append(new DataHolder);
                data[0]->loadData(fileUrl[i]);
            }
        }

        setupSignalBoxes();
    }
}

//Acts if "Exit" is selected in menu. Exits application.
void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

//Acts if plot selector for top plot is changed.
//Replots curves.
void MainWindow::on_signalBox1_currentIndexChanged(int index)
{
    ui->plot1->detachItems();
    indexFix(index, plot1Index, plot1Data);
    setupTraceList(ui->traceList1, plot1Index, plot1Data);
    prepCurves(ui->plot1, plot1Index, plot1Data);
}

//Acts if plot selector for middle plot is changed.
//Replots curves.
void MainWindow::on_signalBox2_currentIndexChanged(int index)
{
    ui->plot2->detachItems();
    indexFix(index, plot2Index, plot2Data);
    setupTraceList(ui->traceList2, plot2Index, plot2Data);
    prepCurves(ui->plot2, plot2Index, plot2Data);
}

//Acts if plot selector for bottom plot is changed.
//Replots curves.
void MainWindow::on_signalBox3_currentIndexChanged(int index)
{
    ui->plot3->detachItems();
    indexFix(index, plot3Index, plot3Data);
    setupTraceList(ui->traceList3, plot3Index, plot3Data);
    prepCurves(ui->plot3, plot3Index, plot3Data);
}

//Acts if an area is selected on top plot.
void MainWindow::onAreaSelected1(const QRectF &pos)
{
    operationSelector(1, pos);
}

//Acts if an area is selected on middle plot.
void MainWindow::onAreaSelected2(const QRectF &pos)
{
    operationSelector(2, pos);
}

//Acts if an area is selected on bottom plot.
void MainWindow::onAreaSelected3(const QRectF &pos)
{
    operationSelector(3, pos);
}

//Acts when given the lists from the signals/operations selector.
void MainWindow::onListIndexesRecieved(QList<QListWidgetItem *> tempSignalList,
                                       QList<QListWidgetItem *>
                                       tempOperationList,
                                       QList<bool> tempTracesSelected)
{
    signalList = tempSignalList;
    operationList = tempOperationList;
    temp = tempTracesSelected;
}

//Acts when "Open files to combine" is selected.
void MainWindow::on_actionOpen_Files_to_Combine_triggered()
{
    QFileDialog openDialog;

    QStringList fileUrl
            = openDialog.getOpenFileNames(this,
                                          tr("Open File"), "" ,
                                          tr("Text Files (*.txt, *.atf)"
                                             ";;Empty Files (*)"));

    if (!fileUrl.isEmpty()) {
        reset();

        for (int i = 0; i < fileUrl.length(); i++) {
            if (data.length() > 0) {
                bool loaded = false;
                for (int j = 0; j < data.length(); j++) {
                    if (fileUrl[i] == NULL
                            || data[j]->fileName() == fileUrl[i]) {
                        loaded = true;
                    }
                }
                if (!loaded) {
                    data.append(new DataHolder);

                    data[data.length()-1]->loadData(fileUrl[i]);

                    loaded = false;
                }
            } else {
                data.append(new DataHolder);
                data[0]->loadData(fileUrl[i]);
            }
        }

        bool match = true;
        for (int i = 0; i < data.length(); i++) {
            for (int j = i; j < data.length(); j++) {
                if (data[i]->header() != data[j]->header()
                        || data[i]->signalHeader() != data[j]->signalHeader()
                        || data[i]->times()[0].length()
                        != data[j]->times()[0].length()) {
                    match = false;
                    QMessageBox failureWarning(QMessageBox::Warning,
                                               "Cannot Combine Files",
                                               "Files were not similar"
                                               "enough to combine.",
                                               QMessageBox::Ok, this);
                    failureWarning.exec();
                    break;
                }
            }
        }
        if (match) {
            for (int i = data.length()-1; i > 0; i--) {
                data[0]->combine(*data[i]);
                data.removeLast();
            }
        }

        setupSignalBoxes();
    }
}

//Acts when "Save files" is selected.
void MainWindow::on_actionSave_Data_triggered()
{
    QFileDialog saveDialog;
    saveData(saveDialog.getSaveFileName(this,
                                        "Save Data...",
                                        "" ,
                                        tr("Axon Text File (*.atf)")));

}

//Acts when an item on the first trace list is changed.
void MainWindow::on_traceList1_itemChanged(QListWidgetItem *item)
{
    QList<bool> temp = data[plot1Data]->tracesActive();
    temp[(item->data(Qt::UserRole).toInt())] = item->checkState();
    data[plot1Data]->setTracesActive(temp);
    setupAllTraceLists();
    refreshCurves();
}

//Acts when an item on the second trace list is changed.
void MainWindow::on_traceList2_itemChanged(QListWidgetItem *item)
{
    QList<bool> temp = data[plot2Data]->tracesActive();
    temp[(item->data(Qt::UserRole).toInt())] = item->checkState();
    data[plot2Data]->setTracesActive(temp);
    setupAllTraceLists();
    refreshCurves();
}

//Acts when an item on the third trace list is changed.
void MainWindow::on_traceList3_itemChanged(QListWidgetItem *item)
{
    QList<bool> temp = data[plot3Data]->tracesActive();
    temp[(item->data(Qt::UserRole).toInt())] = item->checkState();
    data[plot3Data]->setTracesActive(temp);
    setupAllTraceLists();
    refreshCurves();
}
