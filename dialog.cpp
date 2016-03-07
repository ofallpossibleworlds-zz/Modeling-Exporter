#include <QPushButton>
#include "dialog.h"
#include "ui_dialog.h"

//Constructor.
Dialog::Dialog(const DataHolder & data,
               int signalNum, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    dataCopy(data),
    signalNumCopy(signalNum),
    signalSelected(false),
    operationSelected(false),
    startRowCopy(0),
    endRowCopy(data.times()[0].length())
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->signalList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->operationList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    setWindowTitle("Please make a selection.");

    ui->operationList->addItem("Prepare for export.");
    ui->operationList->addItem("Find peaks.");
    ui->operationList->addItem("Find average.");
    ui->operationList->addItem("Find voltages.");
}

Dialog::Dialog(const DataHolder &data,
               int signalNum,
               int startRow,
               int endRow,
               QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    dataCopy(data),
    signalNumCopy(signalNum),
    signalSelected(false),
    operationSelected(false),
    startRowCopy(startRow),
    endRowCopy(endRow)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->signalList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->operationList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    setWindowTitle("Please make a selection.");

    ui->operationList->addItem("Prepare for export.");
    ui->operationList->addItem("Find peaks.");
    ui->operationList->addItem("Find average.");
    ui->operationList->addItem("Find voltages.");
}

//Destructor.
Dialog::~Dialog()
{
    delete ui;
}

//Adds signals to boxes.
void Dialog::addSignals(QStringList signal) {
    ui->signalList->addItems(signal);
}

//Activates when "Ok" is pressed.
//
//This function activates the trace selector. Note that it's possible for this
//function to still return a rejection signal if the trace selector is
//cancelled.
void Dialog::on_buttonBox_accepted()
{

    QList<QListWidgetItem*> signalList;
    signalList = ui->signalList->selectedItems();
    for (int i = 0; i < signalList.length(); i++) {
        QString temp = signalList[i]->text();
        for (int i = 0; i < dataCopy.signalHeader().length(); i++) {
            if (temp == dataCopy.signalHeader()[i]) {
                QString title = "Select traces to operate on for ";
                title = title + dataCopy.signalHeader()[i] + ".";
                TracePickDialog tracePicker(dataCopy,
                                            i,
                                            title,
                                            startRowCopy,
                                            endRowCopy);
                connect(&tracePicker,
                        SIGNAL(tracesSelected(QList<bool>)),
                        this,
                        SLOT(onTracesSelected(QList<bool>)));
                if (tracePicker.exec() == QDialog::Rejected) {
                    on_buttonBox_rejected();
                }
                disconnect(&tracePicker,
                           SIGNAL(tracesSelected(QList<bool>)),
                           this,
                           SLOT(onTracesSelected(QList<bool>)));
            }
        }
    }

    emit listIndexes(ui->signalList->selectedItems(),
                     ui->operationList->selectedItems(),
                     dataCopy.tracesActive());
    done(QDialog::Accepted);
}

//Activates when "Cancel" is pressed, or the window is closed.
void Dialog::on_buttonBox_rejected()
{
    done(QDialog::Rejected);
}

//Activates when the selection on the signal list changes.
//
//This function works in tandem with the operations list selector, and will
//only activate the accept button if there's at least one selection on both.
void Dialog::on_signalList_itemSelectionChanged()
{
    if (ui->signalList->selectedItems().isEmpty()) {
        signalSelected = false;
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    } else {
        signalSelected = true;
        if (operationSelected) {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        }
    }
}

//Activates when the selection on the operation list changes.
//
//This function works in tandem with the signals list selector, and will
//only activate the accept button if there's at least one selection on both.
void Dialog::on_operationList_itemSelectionChanged()
{
    if (ui->operationList->selectedItems().isEmpty()) {
        operationSelected = false;
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    } else {
        operationSelected = true;
        if (signalSelected) {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        }
    }
}

//Activates when the active trace selection is selected, and sets the
//active traces in the data copy to match.
void Dialog::onTracesSelected(QList<bool> activeTraces)
{
    dataCopy.setTracesActive(activeTraces);
}
