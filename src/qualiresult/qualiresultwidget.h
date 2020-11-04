//
// qualiresultwidget.h is part of QualificationMatrix
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

#ifndef QUALIRESULTWIDGET_H
#define QUALIRESULTWIDGET_H

#include <QWidget>

#include <memory>

// Forward declaration for faster compiling.
class QualiResultModel;

class QSqlRelationalTableModel;

class QSortFilterProxyModel;

class QProgressDialog;

class QSqlTableModel;

class QPrinter;

namespace Ui
{
class QualiResultWidget;
}

/**
 * @brief Display the results of qualification.
 * @author Christian Kr, Copyright (c) 2020
 */
class QualiResultWidget: public QWidget
{
Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent
     */
    explicit QualiResultWidget(QWidget *parent = nullptr);

    ~QualiResultWidget() override;

public slots:

    /**
     * @brief Prints the qualification result to a pdf file.
     */
    void printToPDF();

    /**
     * @brief Save table to csv file.
     */
    void saveToCsv();

    /**
     * @brief Update calculation with all filters.
     */
    void updateFilterAndCalculate();

    /**
     * @brief Update the models, cause they might have changed.
     */
    void updateData();

    /**
     * @brief Show or hide filter elements.
     */
    void switchFilterVisibility();

    /**
     * @brief Empty the text in all filters.
     */
    void resetFilter();

    /**
     * @brief Load settings for ui.
     */
    void loadSettings();

    /**
     * @brief Resets the model.
     */
    void resetModel();

    /**
     * @brief Called when the visibility of the filter dock widget changed.
     */
    void filterVisibilityChanged();

private:
    /**
     * @brief Create printer for pdf.
     * @param printer The printer object to draw an.
     */
    void paintPdfRequest(QPrinter *printer);

    Ui::QualiResultWidget *ui;

    std::shared_ptr<QualiResultModel> qualiResultModel;
    std::shared_ptr<QSqlRelationalTableModel> funcModel;
    std::shared_ptr<QSqlRelationalTableModel> trainModel;
    std::shared_ptr<QSqlRelationalTableModel> employeeModel;
    std::shared_ptr<QSqlTableModel> employeeGroupModel;
    std::shared_ptr<QSqlTableModel> trainDataStateModel;

    QSortFilterProxyModel *qualiResultFilterTRState;
    QSortFilterProxyModel *qualiResultFilterTState;
};

#endif // QUALIRESULTWIDGET_H
