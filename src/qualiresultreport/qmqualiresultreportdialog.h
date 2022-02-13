// qmqualiresultreportdialog.h is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.

#ifndef QMQUALIRESULTREPORTDIALOG_H
#define QMQUALIRESULTREPORTDIALOG_H

#include "framework/qmdialog.h"

#include <memory>

class QSqlTableModel;
class QMSqlTableModel;
class QSqlRelationalTableModel;
class QPrinter;
class QMQualiResultReportItem;

QT_BEGIN_NAMESPACE
namespace Ui
{
class QMQualiResultReportDialog;
}
QT_END_NAMESPACE

/// Sett different proprties to start creating a quali result report.
/// \author Christian Kr, Copyright 2022
class QMQualiResultReportDialog: public QMDialog
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object of the widget. This is important for a modal dialog.
    explicit QMQualiResultReportDialog(QWidget *parent = nullptr);

    /// Destructor
    ~QMQualiResultReportDialog() override;

    /// Override from QMDialog.
    void saveSettings() override;

    /// Override from QDialog.
    void loadSettings() override;

    /// Update the list information.
    void updateInfo();

public slots:
    /// Add all selected trainings.
    [[maybe_unused]] void addTrainings();

    /// Remove selected trainings in selection dialog.
    [[maybe_unused]] void removeTrainings();

    /// Add all trainings from selected training groups.
    [[maybe_unused]] void addFromTrainingGroups();

    /// Remove all selected trainings from training group.
    [[maybe_unused]] void removeFromTrainingGroups();

    /// Clear the trainings list and the internal hash list.
    [[maybe_unused]] void resetTrainingList();

    /// Create the report.
    [[maybe_unused]] void createReport();

    // Paint request for creating the pdf.
    void paintPdfRequest(QPrinter *printer);

private:
    /// Calculate the whole result and return it as a list.
    QList<QMQualiResultReportItem> calculateResult();

    Ui::QMQualiResultReportDialog *ui;

    std::unique_ptr<QHash<int, QString>> trainList;
};

#endif // QMQUALIRESULTREPORTDIALOG_H
