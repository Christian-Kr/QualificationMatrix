// qmtraindatawidget.h is part of QualificationMatrix
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

#ifndef QMTRAINDATAWIDGET_H
#define QMTRAINDATAWIDGET_H

#include "framework/component/qmwinmodewidget.h"

#include <memory>

class QMSqlTableModel;
class QSqlRelationalTableModel;
class QSqlTableModel;
class QProgressDialog;
class QItemSelection;
class QMTrainingDataModel;

namespace Ui
{
class QMTrainDataWidget;
}

/// Widget for training data to manipulate.
/// \author Christian Kr, Copyright 2020
class QMTrainDataWidget: public QMWinModeWidget
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit QMTrainDataWidget(QWidget *parent = nullptr);

    /// Destructor
    ~QMTrainDataWidget() override;

    /// Update the table view. Meaning to set hidden columns and delegators.
    void updateTableView();

public slots:
    /// Load settings.
    void loadSettings();

    /// Save settings.
    void saveSettings();

    /// Update the models, cause they might have changed.
    void updateData();

    /// Update table with filter from ui components.
    void updateFilter();

    /// Reset the content of the filter.
    void resetFilter();

    /// Add single entry into train data model.
    void addSingleEntry();

    /// Delete selected train data elements.
    void deleteSelected();

    /// Show the import csv dialog.
    void importCsv();

    /// Show certificates in train data docked widget.
    void showTrainDataCertificates();

    /// Show a widget for multi editing entries.
    void showMultiEdit();

    /// Selection of trainDataTable changed.
    /// \param deselected
    /// \param selected
    void trainDataSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    /// Add a certificate to a train data entry.
    void addCertificate();

    /// Remove a certificate from a train data entry.
    void removeCertificate();

    /// Show selected certificate.
    void showCert();

    /// Set the training filter.
    /// \param training
    void setTrainFilter(QString training);

    /// Set name filter.
    /// \param name
    void setNameFilter(QString name);

    /// Clear filtering dates.
    void clearDates();

    /// Update which item is enabled and which not on dock widget for multi edit.
    void updateMultiEditEnabledState();

    /// Execute the multi edit process.
    void executeMultiEdit();

    /// Add multiple entries.
    void addMultipleEntries();

    /// Choose a certificate and return the id.
    /// \return The id if one has been selected, else -1.
    int chooseCertificate();

    /// Choose a certificate and set it to multi edit dialog.
    void chooseMultiCertificate();

signals:
    /// There is a short message that should be displayed.
    /// \param msg
    void infoMessageAvailable(QString msg);

    /// There is a short warn message that should be displayed.
    /// \param msg
    void warnMessageAvailable(QString msg);

private:
    Ui::QMTrainDataWidget *ui;

    std::unique_ptr<QSqlTableModel> employeeViewModel;
    std::unique_ptr<QSqlTableModel> trainViewModel;
    std::unique_ptr<QMTrainingDataModel> trainDataModel;
    std::unique_ptr<QSqlTableModel> trainDataStateViewModel;
    std::unique_ptr<QSqlTableModel> trainDataCertModel;
    std::unique_ptr<QSqlTableModel> trainDataCertViewModel;

    int multiEditCertiId;
};

#endif // QMTRAINDATAWIDGET_H
