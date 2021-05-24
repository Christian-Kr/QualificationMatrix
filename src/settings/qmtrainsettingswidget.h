// qmtrainsettingswidget.cpp is part of QualificationMatrix
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

#ifndef QMTRAINSETTINGSWIDGET_H
#define QMTRAINSETTINGSWIDGET_H

#include "qmsettingswidget.h"

#include <memory>
#include <QSortFilterProxyModel>

class QMSqlRelationalTableModel;
class QSqlRelationalTableModel;
class QSqlTableModel;

namespace Ui
{
class QMTrainSettingsWidget;
}

/// Shows a settings widget for training data.
/// \author Christian Kr, Copyright (c) 2020
class QMTrainSettingsWidget: public QMSettingsWidget
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit QMTrainSettingsWidget(QWidget *parent = nullptr);

    /// Destructor
    ~QMTrainSettingsWidget() override;

    /// This slot has to be implemented by parent class. When calling this function, it should take a QSettings object
    /// or data Model (bspw. SqlModel) to save every changes to a certificate based system.
    void saveSettings() override;

    ///  Override from QMSettingsWidget
    void revertChanges() override;

    /// Override from QMSettingsWidget.
    void loadSettings() override;

    /// Update the table view like hiding columns.
    void updateTableView();

public slots:
    /// Updates all data that exist are needed.
    void updateData();

    /// Add a new training.
    void addTrain();

    /// Remove a selected training.
    void removeTrain();

    /// Revert changes in training data model.
    void revertTrain();

    /// Add a new training group.
    void addTrainGroups();

    /// Remove a selected training group.
    void removeTrainGroups();

    /// Revert changes in training group data model.
    void revertTrainGroups();

    /// Filter the training table.
    void filterTrain();

    /// Remove a training date state.
    void removeTrainState();

    /// Add a new training data state.
    void addTrainState();

    // Revert changes to all training data states.
    void revertTrainStates();

private:
    /// Search for references in models to the given training.
    /// \param func Name of the training to search for references.
    /// \return True if reference found, else false.
    bool trainReference(const QString &train);

    Ui::QMTrainSettingsWidget *ui;

    std::unique_ptr<QMSqlRelationalTableModel> trainModel;
    std::unique_ptr<QSqlTableModel> trainGroupModel;
    std::unique_ptr<QSqlTableModel> trainDataStateModel;
    std::unique_ptr<QSqlTableModel> trainDataViewModel;

    QSortFilterProxyModel *trainFilterModel;
};

#endif // QMTRAINSETTINGSWIDGET_H
