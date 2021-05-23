// qmqualiresultwidget.h is part of QualificationMatrix
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

#ifndef QMQUALIRESULTWIDGET_H
#define QMQUALIRESULTWIDGET_H

#include <QWidget>

#include <memory>

class QMQualiResultModel;
class QMSqlRelationalTableModel;
class QSqlRelationalTableModel;
class QSortFilterProxyModel;
class QProgressDialog;
class QSqlTableModel;
class QPrinter;

namespace Ui
{
class QMQualiResultWidget;
}

/// Display the results of qualification.
/// \author Christian Kr, Copyright 2020
class QMQualiResultWidget: public QWidget
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit QMQualiResultWidget(QWidget *parent = nullptr);

    /// Destructor
    ~QMQualiResultWidget() override;

public slots:
    /// Prints the qualification result to a pdf certificate.
    void printToPDF();

    /// Save table to csv certificate.
    void saveToCsv();

    /// Update calculation with all filters.
    void updateFilterAndCalculate();

    /// Update the models, cause they might have changed.
    void updateData();

    /// Show or hide filter elements.
    void switchFilterVisibility();

    /// Empty the text in all filters.
    void resetFilter();

    /// Load settings for ui.
    void loadSettings();

    /// Save settings
    void saveSettings();

    /// Resets the model.
    void resetModel();

    /// Called when the visibility of the filter dock widget changed.
    void filterVisibilityChanged();

    /// Reset func filter box.
    void resetFunc();

    /// Reset train filter box.
    void resetTrain();

    /// Reset train state filter box.
    void resetTrainState();

    /// Reset train result state filter box.
    void resetTrainResultState();

    /// Reset func employee box.
    void resetEmployee();

    /// Reset employee group filter box.
    void resetEmployeeGroup();

    /// Show extended filter dialog for employee.
    void extSelEmployee();

    /// Show extended filter dialog for employee group.
    void extSelEmployeeGroup();

    /// Show extended filter dialog for train result state.
    void extSelTrainResultState();

    /// Show extended filter dialog for train data state.
    void extSelTrainDataState();

    /// Reaction when there is a double click on an element.
    void onDoubleClick(const QModelIndex index);

signals:
    /// Emited when the corresponding train data entry should be shown.
    void showTrainData(QString name, QString training);

private:
    /// Create printer for pdf.
    /// \param printer The printer object to draw an.
    void paintPdfRequest(QPrinter *printer);

    Ui::QMQualiResultWidget *ui;

    std::shared_ptr<QMQualiResultModel> qualiResultModel;
    std::unique_ptr<QSqlTableModel> funcViewModel;
    std::unique_ptr<QSqlTableModel> trainViewModel;
    std::unique_ptr<QSqlRelationalTableModel> employeeViewModel;
    std::shared_ptr<QSqlTableModel> employeeGroupModel;
    std::shared_ptr<QSqlTableModel> trainDataStateModel;

    QSortFilterProxyModel *qualiResultFilterTRState;
    QSortFilterProxyModel *qualiResultFilterTState;
};

#endif // QMQUALIRESULTWIDGET_H
