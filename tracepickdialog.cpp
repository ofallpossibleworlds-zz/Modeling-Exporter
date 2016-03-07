#include "tracepickdialog.h"
#include "ui_tracepickdialog.h"

//Constructor.
TracePickDialog::TracePickDialog(const DataHolder& data,
                                 int signalNum,
                                 QString windowTitle,
                                 QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TracePickDialog),
    dataCopy(data),
    signalNumCopy(signalNum),
    traceSelection(data.tracesActive()),
    startRowCopy(0),
    endRowCopy(data.times()[0].length())
{
    ui->setupUi(this);
    setWindowTitle(windowTitle);
    prepPlot();
    addSignals();
}

TracePickDialog::TracePickDialog(const DataHolder &data,
                                 int signalNum,
                                 QString windowTitle,
                                 int startRow,
                                 int endRow,
                                 QWidget *parent,
                                 bool multiSelect,
                                 int* traceNum) :
    QDialog(parent),
    ui(new Ui::TracePickDialog),
    dataCopy(data),
    signalNumCopy(signalNum),
    traceSelection(data.tracesActive()),
    startRowCopy(startRow),
    endRowCopy(endRow),
    multiSelectCopy(multiSelect),
    traceNumCopy(traceNum)
{
    ui->setupUi(this);
    setWindowTitle(windowTitle);
    prepPlot();
    addSignals();

    if (!multiSelectCopy) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}

//Destructor.
TracePickDialog::~TracePickDialog()
{
    delete ui;
}

//Acts if "Ok" is selected.
void TracePickDialog::on_buttonBox_accepted()
{
    if (multiSelectCopy) {
        emit tracesSelected(traceSelection);
    }
    done(QDialog::Accepted);
}

//Acts if "Cancel" is selected or the window is closed.
void TracePickDialog::on_buttonBox_rejected()
{
    done(QDialog::Rejected);
}

//Acts if an item on the trace list is changed.
void TracePickDialog::on_traceList_itemChanged(QListWidgetItem *item)
{
    if (multiSelectCopy) {
        traceSelection[(item->data(Qt::UserRole).toInt())] = item->checkState();
    } else {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        int checkStateSaver = item->checkState();
        int counter = 0;
        for (int i = 1; i < dataCopy.traces().length(); i++) {
            if (dataCopy.signal()[i].contains(
                        dataCopy.signalHeader()[signalNumCopy])) {
                if (ui->traceList->item(counter) != item) {
                    traceSelection[i] = false;
                    ui->traceList->item(counter)->setCheckState(Qt::Unchecked);
                }
                counter++;
            }
        }
        traceSelection[(item->data(Qt::UserRole).toInt())] = checkStateSaver;
        if (checkStateSaver == 0) {
            item->setCheckState(Qt::Unchecked);
        } else {
            item->setCheckState(Qt::Checked);
            *traceNumCopy = item->data(Qt::UserRole).toInt();
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        }
    }
    prepCurves();
}


//Adds signals to the list.
void TracePickDialog::addSignals()
{
    ui->traceList->clear();
    for (int i = 1; i < dataCopy.traces().length(); i++) {
        if (dataCopy.signal()[i].contains(
                    dataCopy.signalHeader()[signalNumCopy])) {
            QListWidgetItem* temp = new QListWidgetItem(dataCopy.traces()[i]);
            temp->setFlags(temp->flags() | Qt::ItemIsUserCheckable);
            if (multiSelectCopy) {
                if (dataCopy.tracesActive()[i]) {
                    temp->setCheckState(Qt::Checked);
                } else {
                    temp->setCheckState(Qt::Unchecked);
                }
            } else {
                temp->setCheckState(Qt::Unchecked);
            }
            temp->setData(Qt::UserRole, QVariant(i));
            ui->traceList->addItem(temp);
        }
    }
    prepCurves();
}

//Prepares the plot to display curves.
void TracePickDialog::prepPlot()
{
    //Makes background gray.
    ui->plot->setCanvasBackground(Qt::lightGray);

    //Sets axes to auto-scale to data.
    ui->plot->setAxisAutoScale(0, true);
    ui->plot->setAxisAutoScale(1, true);

    //Shows plot with changes.
    ui->plot->replot();
}

//Prepares the curves and displays them on the plot.
void TracePickDialog::prepCurves()
{
    prepCurves(0, dataCopy.times()[0].length() -1);
}

void TracePickDialog::prepCurves(int startRow, int endRow)
{
    ui->plot->detachItems();
    //Creates list to hold curves.
    QList<QwtPlotCurve*> curveList;

    //Sets up curves, and stores points from DataHolder.
    int counter = 0;

    for (int i = 1; i < dataCopy.signal().length(); i++) {
        if (dataCopy.signal()[i].contains(
                    dataCopy.signalHeader()[signalNumCopy])) {
                if (traceSelection[i]) {
                    curveList.append(new QwtPlotCurve());
                    curveList[counter]->setTitle("");
                    curveList[counter]->setPen(Qt::red, 2);
                    curveList[counter]->setRenderHint(QwtPlotItem::RenderAntialiased,
                                                      true);
                } else {
                    curveList.append(new QwtPlotCurve());
                    curveList[counter]->setTitle("");
                    curveList[counter]->setPen(Qt::black, 1);
                    curveList[counter]->setRenderHint(QwtPlotItem::RenderAntialiased,
                                                      true);
                }

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

QPolygonF TracePickDialog::loadPrecisePoints(int col, int startRow, int endRow)
{
    QPolygonF points;
    for (int i = startRow;
         i < dataCopy.times()[0].length() - 1 && i < endRow;
         i++) {
        points << QPointF(dataCopy.times()[0][i], dataCopy.times()[col][i]);
    }
    return points;
}

QPolygonF TracePickDialog::loadApproximatePoints(int col,
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

void TracePickDialog::on_checkBox_toggled(bool checked)
{
    if (checked) {
        prepCurves(startRowCopy, endRowCopy);
    } else {
        prepCurves();
    }
}
