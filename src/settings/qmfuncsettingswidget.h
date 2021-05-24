// qmfunctionwidget.h is part of QualificationMatrix
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

#ifndef QMFUNCSETTINGSWIDGET_H
#define QMFUNCSETTINGSWIDGET_H

#include "qmsettingswidget.h"

#include <memory>
#include <QSortFilterProxyModel>

class QMSqlRelationalTableModel;
class QSqlRelationalTableModel;
class QSqlTableModel;

namespace Ui
{
class QMFuncSettingsWidget;
}

/// Shows a settings widget for function data.
/// \author Christian Kr, Copyright (c) 2020
class QMFuncSettingsWidget: public QMSettingsWidget
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit QMFuncSettingsWidget(QWidget *parent = nullptr);

    /// Destructor
    ~QMFuncSettingsWidget() override;

    /// This slot has to be implemented by parent class. When calling this function, it should take a QSettings object
    /// or data Model (bspw. SqlModel) to save every changes to a certificate based system.
    void saveSettings() override;

    /// Override from QMSettingsWidget
    void revertChanges() override;

    /// Override from QMSettingsWidget.
    void loadSettings() override;

    /// Update table view like setting hidden columns.
    void updateTableView();

public slots:
    /// Updates all data that exist are needed.
    void updateData();

    /// Add a new function.
    void addFunc();

    /// Remove a selected function.
    void removeFunc();

    /// Revert changes in function data model.
    void revertFunc();

    /// Add a new function group.
    void addFuncGroups();

    /// Remove a selected function group.
    void removeFuncGroups();

    /// Revert changes in function group data model.
    void revertFuncGroups();

    /// Filter the function table.
    void filterFunc();

private:
    /// Search for references in models to the given function.
    /// \param func Name of the function to search for references.
    /// \return True if reference found, else false.
    bool funcReference(const QString &func);

    Ui::QMFuncSettingsWidget *ui;

    std::unique_ptr<QMSqlRelationalTableModel> funcModel;
    std::unique_ptr<QSqlTableModel> funcGroupModel;

    QSortFilterProxyModel *funcFilterModel;
};

#endif // QMFUNCSETTINGSWIDGET_H
