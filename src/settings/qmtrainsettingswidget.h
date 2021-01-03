//
// qmtrainsettingswidget.cpp is part of QualificationMatrix
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

#ifndef QMTRAINSETTINGSWIDGET_H
#define QMTRAINSETTINGSWIDGET_H

#include "qmsettingswidget.h"

#include <memory>
#include <QSortFilterProxyModel>

// Forward declaration for faster compiling.
class QSqlRelationalTableModel;
class QSqlTableModel;

namespace Ui
{
class QMTrainSettingsWidget;
}

/**
 * @brief Shows a settings widget for training data.
 * @author Christian Kr, Copyright (c) 2020
 */
class QMTrainSettingsWidget: public QMSettingsWidget
{
Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent
     */
    explicit QMTrainSettingsWidget(QWidget *parent = nullptr);

    ~QMTrainSettingsWidget() override;

    /**
     * @brief This slot has to be implemented by parent class.
     *
     * When calling this function, it should take a QSettings object or data Model
     * (bspw. SqlModel) to save every changes to a certificate based system.
     */
    void saveSettings() override;

    /**
     * @brief Override from QMSettingsWidget
     */
    void revertChanges() override;

    /// Override from QMSettingsWidget.
    void loadSettings() override;

public slots:

    /**
     * @brief Updates all data that exist are needed.
     */
    void updateData();

    /**
     * @brief Add a new training.
     */
    void addTrain();

    /**
     * @brief Remove a selected training.
     */
    void removeTrain();

    /**
     * @brief Revert changes in training data model.
     */
    void revertTrain();

    /**
     * @brief Add a new training group.
     */
    void addTrainGroups();

    /**
     * @brief Remove a selected training group.
     */
    void removeTrainGroups();

    /**
     * @brief Revert changes in training group data model.
     */
    void revertTrainGroups();

    /**
     * @brief Filter the training table.
     */
    void filterTrain();

    /// Remove a training date state.
    void removeTrainState();

    /// Add a new training data state.
    void addTrainState();

    // Revert changes to all training data states.
    void revertTrainStates();

private:
    /**
     * @brief Search for references in models to the given training.
     * @param func Name of the training to search for references.
     * @return True if reference found, else false.
     */
    bool trainReference(const QString &train);

    Ui::QMTrainSettingsWidget *ui;

    std::shared_ptr<QSqlRelationalTableModel> trainModel;
    std::shared_ptr<QSqlTableModel> trainGroupModel;
    std::shared_ptr<QSqlTableModel> trainDataStateModel;
    std::shared_ptr<QSqlRelationalTableModel> trainDataModel;

    QSortFilterProxyModel *trainFilterModel;
};

#endif // QMTRAINSETTINGSWIDGET_H
