//
// qmtraindatadetailsdialog.h is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
// the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.
//

#ifndef QMTRAINDATADETAILSDIALOG_H
#define QMTRAINDATADETAILSDIALOG_H

#include <QDialog>

namespace Ui
{
class QMTrainDataDetailsDialog;
}

class QMTrainDataDetailsDialog: public QDialog
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit QMTrainDataDetailsDialog(QWidget *parent = nullptr);

    /// Destructor
    ~QMTrainDataDetailsDialog() override;

    /// Save settings of the dialog before it closes.
    void saveSettings();

    /// Override from QDialog.
    /// \param event
    void closeEvent(QCloseEvent *event) override;

    /// Override from QDialog.
    void accept() override;

    /// Override from QDialog.
    void reject() override;

public slots:
    /// The apply button has been pressed.
    void apply();

protected:
    /// Override from QDialog.
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::QMTrainDataDetailsDialog *ui;
};

#endif // QMTRAINDATADETAILSDIALOG_H
