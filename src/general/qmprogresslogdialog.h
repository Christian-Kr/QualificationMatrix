#ifndef QMPROGRESSLOGDIALOG_H
#define QMPROGRESSLOGDIALOG_H

#include <QDialog>

namespace Ui
{
class QMProgressLogDialog;
}
class QMProgressLogDialog: public QDialog
{
Q_OBJECT

public:
    explicit QMProgressLogDialog(QWidget *parent = nullptr);
    ~QMProgressLogDialog();

private:
    Ui::QMProgressLogDialog *ui;
};

#endif // QMPROGRESSLOGDIALOG_H
