//
// traindatadialog.h is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with QualificationMatrix.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef TRAINDATADIALOG_H
#define TRAINDATADIALOG_H

#include <QDialog>

namespace Ui
{
class TrainDataDialog;
}

class TrainDataDialog: public QDialog
{
Q_OBJECT

public:
    /**
     * @brief Constructor.
     */
    explicit TrainDataDialog(QWidget *parent = nullptr);

    ~TrainDataDialog();

    /**
     * @brief Save settings of the dialog before it closes.
     */
    void saveSettings();

    /**
     * @brief Override from QDialog.
     */
    virtual void closeEvent(QCloseEvent *event) override;

    /**
     * @brief Override from QDialog.
     */
    virtual void accept() override;

    /**
     * @brief Override from QDialog.
     */
    virtual void reject() override;

public slots:

    /**
     * @brief The apply button has been pressed.
     */
    void apply();

private:
    Ui::TrainDataDialog *ui;
};

#endif // TRAINDATADIALOG_H
