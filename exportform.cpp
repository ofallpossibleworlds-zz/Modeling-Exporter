#include <QPushButton>

#include "exportform.h"
#include "ui_exportform.h"

ExportForm::ExportForm(int* voltage,
                       int* index,
                       QString* title,
                       QWidget *parent):
    QDialog(parent),
    ui(new Ui::ExportForm),
    voltageHolder(voltage),
    indexHolder(index),
    titleHolder(title)
{
    ui->setupUi(this);
    ui->voltageSelectorBox->setValue(*voltageHolder);
    *indexHolder = ui->variantSelector->value();

    setWindowTitle("Prepare to export...");

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

ExportForm::~ExportForm()
{
    delete ui;
}

void ExportForm::on_buttonBox_accepted()
{
    done(QDialog::Accepted);
}

void ExportForm::on_buttonBox_rejected()
{
    done(QDialog::Rejected);
}

void ExportForm::on_nameSelector_textChanged(const QString &arg1)
{
    *titleHolder = arg1;
    if (titleHolder->isEmpty()) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    } else {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

void ExportForm::on_variantSelector_valueChanged(int arg1)
{
    *indexHolder = arg1;
}

void ExportForm::on_voltageSelectorBox_valueChanged(int arg1)
{
    *voltageHolder = arg1;
}
