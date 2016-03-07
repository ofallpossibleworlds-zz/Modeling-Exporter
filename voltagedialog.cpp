#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include "voltagedialog.h"
#include "ui_voltagedialog.h"

//Constructor.
VoltageDialog::VoltageDialog(const DataHolder &data,
                             QString title,
                             QString helpText,
                             float *voltage,
                             int signalNum,
                             int traceNum,
                             int startRow,
                             int endRow,
                             QWidget *parent):
    QDialog(parent),
    ui(new Ui::VoltageDialog),
    dataCopy(data),
    voltageRef(voltage),
    signalNumCopy(signalNum),
    traceNumCopy(traceNum),
    startRowCopy(startRow),
    endRowCopy(endRow),
    returnedSignalCopy(nullptr),
    selectorTypeCopy(0)
{
    ui->setupUi(this);

    setWindowTitle(title);
    ui->infoPanel->setText(helpText);

    setupPlotSelector();

    prepPlot();

    prepCurves();
}

VoltageDialog::VoltageDialog(const DataHolder &data,
                             QString title,
                             QString helpText,
                             float *voltage,
                             int signalNum,
                             int startRow,
                             int endRow,
                             QWidget *parent):
    QDialog(parent),
    ui(new Ui::VoltageDialog),
    dataCopy(data),
    voltageRef(voltage),
    signalNumCopy(signalNum),
    startRowCopy(startRow),
    endRowCopy(endRow),
    returnedSignalCopy(nullptr),
    selectorTypeCopy(2)
{
    ui->setupUi(this);

    setWindowTitle(title);
    ui->infoPanel->setText(helpText);

    setupPlotSelector();

    prepPlot();

    prepCurves();
}

VoltageDialog::VoltageDialog(const DataHolder &data,
                             QString title,
                             QString helpText,
                             float *startTime,
                             float *endTime,
                             int signalNum,
                             int startRow,
                             int endRow,
                             int *returnedSignal,
                             QWidget *parent):
    QDialog(parent),
    ui(new Ui::VoltageDialog),
    dataCopy(data),
    startTimeCopy(startTime),
    endTimeCopy(endTime),
    signalNumCopy(signalNum),
    startRowCopy(startRow),
    endRowCopy(endRow),
    returnedSignalCopy(returnedSignal),
    selectorTypeCopy(1)
{
    ui->setupUi(this);

    setWindowTitle(title);
    ui->infoPanel->setText(helpText);

    setupPlotSelector();

    prepPlot();

    prepCurves();
}



//Destructor.
VoltageDialog::~VoltageDialog()
{
    delete ui;
}

//Prepares the plot.
void VoltageDialog::prepPlot()
{
    //Makes background gray.
    ui->plot->setCanvasBackground(Qt::lightGray);

    if (selectorTypeCopy == 0 || selectorTypeCopy == 2) {
        //Sets up plot selector.
        QwtPlotPicker* selector = new QwtPlotPicker(
                    QwtPlot::xBottom,
                    QwtPlot::yLeft,
                    QwtPlotPicker::CrossRubberBand,
                    QwtPicker::AlwaysOn,
                    ui->plot->canvas());
        selector->setStateMachine(new QwtPickerClickPointMachine());
        selector->setRubberBandPen(QColor(Qt::red));
        selector->setRubberBand(QwtPicker::VLineRubberBand);
        selector->setTrackerPen(QColor(Qt::red));
        connect(selector,
                SIGNAL(selected(QPointF)),
                this,
                SLOT(voltageSelected(QPointF)));
    } else {
        QwtPlotPicker* selector = new QwtPlotPicker(QwtPlot::xBottom,
                                                    QwtPlot::yLeft,
                                                    QwtPlotPicker::
                                                    CrossRubberBand,
                                                    QwtPicker::AlwaysOn,
                                                    ui->plot->canvas());
        selector->setStateMachine(new QwtPickerClickRectMachine());
        selector->setRubberBandPen(QColor(Qt::red));
        selector->setRubberBand(QwtPicker::RectRubberBand);
        selector->setTrackerPen(QColor(Qt::red));
        connect(selector,
                SIGNAL(selected(QRectF)),
                this,
                SLOT(voltageSelected(QRectF)));
    }

    //Makes background gray.
    ui->plot->setCanvasBackground(Qt::lightGray);

    //Sets axes to auto-scale to data.
    ui->plot->setAxisAutoScale(0, true);
    ui->plot->setAxisAutoScale(1, true);

    //Shows plot with changes.
    ui->plot->replot();
}

//Prepares the curves and displays them on the plot.
void VoltageDialog::prepCurves()
{
    prepCurves(0, dataCopy.times()[0].length() -1);
}

void VoltageDialog::prepCurves(int startRow, int endRow)
{
    ui->plot->detachItems();
    //Creates list to hold curves.
    QList<QwtPlotCurve*> curveList;

    //Sets up curves, and stores points from DataHolder.
    int counter = 0;

    for (int i = 1; i < dataCopy.signal().length(); i++) {
        if (dataCopy.signal()[i].contains(
                    dataCopy.signalHeader()[signalNumCopy])
                && dataCopy.tracesActive()[i]) {
            curveList.append(new QwtPlotCurve());
            curveList[counter]->setTitle("");
            if (selectorTypeCopy == 0) {
                if (i == traceNumCopy) {
                    curveList[counter]->setPen(Qt::red, 2);
                } else {
                    curveList[counter]->setPen(Qt::black, 1);
                }
            } else {
                curveList[counter]->setPen(Qt::red, 2);
            }
            curveList[counter]->setRenderHint(QwtPlotItem::RenderAntialiased,
                                              true);

            QPolygonF points;

            //Loads all points if few enough. Loads some otherwise.
            if (dataCopy.times()[0].length() > 1000) {
                points = loadApproximatePoints(i, startRow, endRow);
            } else {
                points = loadPrecisePoints(i, startRow, endRow);
            }

            //Attaches curve.
            curveList[counter]->setSamples(points);
            curveList[counter]->attach(ui->plot);

            counter++;
        }
    }

    //Adds grid.
    QwtPlotGrid* grid1 = new QwtPlotGrid();
    grid1->attach(ui->plot);

    //Shows updated plot.
    ui->plot->replot();
}
/*
//Prepares the curves and displays them on the plot.
void VoltageDialog::prepCurves()
{
    ui->plot->detachItems();
    //Creates list to hold curves.
    QList<QwtPlotCurve*> curveList;

    //Sets up curves, and stores points from DataHolder.
    int counter = 0;

    for (int i = 1; i < dataCopy.signal().length(); i++) {
        if (dataCopy.signal()[i].contains(
                    dataCopy.signalHeader()[signalNumCopy])
                && dataCopy.tracesActive()[i]) {
            curveList.append(new QwtPlotCurve());
            curveList[counter]->setTitle("");
            if (i == traceNumCopy) {
                curveList[counter]->setPen(Qt::red, 2);
            } else {
                curveList[counter]->setPen(Qt::black, 1);
            }
            curveList[counter]->setRenderHint(QwtPlotItem::RenderAntialiased,
                                              true);

            QPolygonF points;

            //Loads all points if few enough. Loads some otherwise.
            if (dataCopy.times()[0].length() > 1000) {
                points = loadApproximatePoints(i, startRow, endRow);
            } else {
                points = loadPrecisePoints(i, startRow, endRow);
            }

            //Attaches curve.
            curveList[counter]->setSamples(points);
            curveList[counter]->attach(ui->plot);

            counter++;
        }
    }

    //Adds grid.
    QwtPlotGrid* grid1 = new QwtPlotGrid();
    grid1->attach(ui->plot);

    //Shows updated plot.
    ui->plot->replot();
}
*/
QPolygonF VoltageDialog::loadPrecisePoints(int col, int startRow, int endRow)
{
    QPolygonF points;
    for (int i = startRow;
         i < dataCopy.times()[0].length() - 1 && i < endRow;
         i++) {
        points << QPointF(dataCopy.times()[0][i], dataCopy.times()[col][i]);
    }
    return points;
}

QPolygonF VoltageDialog::loadApproximatePoints(int col,
                                               int startRow,
                                               int endRow)
{
    QPolygonF points;
    for (int i = startRow; i < dataCopy.times()[0].length() - 1 && i < endRow;
         i = i + (dataCopy.times()[0].length())/1000) {
        points << QPointF(dataCopy.times()[0][i], dataCopy.times()[col][i]);
    }
    return points;
}

void VoltageDialog::setupPlotSelector()
{
    //Adds signals.
    for (int j = 0; j < dataCopy.signalHeader().length(); j++) {
        ui->plotSelector->addItem(dataCopy.signalHeader()[j]);
    }
}

//Acts when a point is selected on the plot representing the
//intended voltage.
void VoltageDialog::voltageSelected(QPointF point)
{
    *voltageRef = ((float)point.x());
    done(QDialog::Accepted);
}

void VoltageDialog::voltageSelected(QRectF rect)
{
    *startTimeCopy = ((float)rect.left());
    *endTimeCopy = ((float)rect.right());
    done(QDialog::Accepted);
}


void VoltageDialog::on_checkBox_toggled(bool checked)
{
    if (checked) {
        prepCurves(startRowCopy, endRowCopy);
    } else {
        prepCurves();
    }
}

void VoltageDialog::on_plotSelector_currentIndexChanged(int index)
{
    signalNumCopy = index;
    if (returnedSignalCopy != nullptr) {
        *returnedSignalCopy = index;
    }
    if (ui->checkBox->isChecked()) {
        prepCurves(startRowCopy, endRowCopy);
    } else {
        prepCurves();
    }
}
