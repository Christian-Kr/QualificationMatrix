#include "qmprogresslogdialog.h"
#include "ui_qmprogresslogdialog.h"

QMProgressLogDialog::QMProgressLogDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::QMProgressLogDialog)
{
    ui->setupUi(this);
}

QMProgressLogDialog::~QMProgressLogDialog()
{
    delete ui;
}
