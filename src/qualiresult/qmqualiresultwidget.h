//
// qmqualiresultwidget.h is part of QualificationMatrix
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

#ifndef QMQUALIRESULTWIDGET_H
#define QMQUALIRESULTWIDGET_H

#include <QWidget>

#include <memory>

// Forward declaration for faster compiling.
class QMQualiResultModel;
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

private:
    /// Create printer for pdf.
    /// \param printer The printer object to draw an.
    void paintPdfRequest(QPrinter *printer);

    Ui::QMQualiResultWidget *ui;

    std::shared_ptr<QMQualiResultModel> qualiResultModel;
    std::shared_ptr<QSqlRelationalTableModel> funcModel;
    std::shared_ptr<QSqlRelationalTableModel> trainModel;
    std::shared_ptr<QSqlRelationalTableModel> employeeModel;
    std::shared_ptr<QSqlTableModel> employeeGroupModel;
    std::shared_ptr<QSqlTableModel> trainDataStateModel;

    QSortFilterProxyModel *qualiResultFilterTRState;
    QSortFilterProxyModel *qualiResultFilterTState;
};

#endif // QMQUALIRESULTWIDGET_H
