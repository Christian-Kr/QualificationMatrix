//
// qmtraindatawidget.h is part of QualificationMatrix
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

#ifndef QMTRAINDATAWIDGET_H
#define QMTRAINDATAWIDGET_H

#include <QWidget>
#include <memory>

// Forward declarations for faster compiling.
class QSqlRelationalTableModel;
class QSqlTableModel;
class QProgressDialog;
class QItemSelection;

namespace Ui
{
class QMTrainDataWidget;
}

/// Widget for training data to manipulate.
/// \author Christian Kr, Copyright 2020
class QMTrainDataWidget: public QWidget
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit QMTrainDataWidget(QWidget *parent = nullptr);

    /// Destructor
    ~QMTrainDataWidget() override;

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

    /// Selection of trainDataTable changed.
    void trainDataSelectionChanged(const QItemSelection &selected,
        const QItemSelection &deselected);

    /// Add a certificate to a train data entry.
    void addCertificate();

    /// Remove a certificate from a train data entry.
    void removeCertificate();

    /// Show selected certificate.
    void showCert();

signals:
    /// There is a short message that should be displayed.
    /// \param msg
    void infoMessageAvailable(QString msg);

    /// There is a short warn message that should be displayed.
    /// \param msg
    void warnMessageAvailable(QString msg);

private:
    Ui::QMTrainDataWidget *ui;

    std::shared_ptr<QSqlRelationalTableModel> employeeModel;
    std::shared_ptr<QSqlRelationalTableModel> trainModel;
    std::shared_ptr<QSqlRelationalTableModel> trainDataModel;
    std::shared_ptr<QSqlTableModel> trainDataStateModel;
    std::shared_ptr<QSqlTableModel> trainDataCertModel;
    std::shared_ptr<QSqlTableModel> trainDataCertViewModel;
};

#endif // QMTRAINDATAWIDGET_H
