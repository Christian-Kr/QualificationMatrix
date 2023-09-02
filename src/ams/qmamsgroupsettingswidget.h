// qmamsgroupsettingswidget.h is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
// the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.

#ifndef QMAMSGROUPSETTINGSWIDGET_H
#define QMAMSGROUPSETTINGSWIDGET_H

#include "settings/qmsettingswidget.h"

class QMAMSGroupModel;
class QMAMSAccessModeModel;
class QMAMSGroupAccessModeModel;
class QMEmployeeModel;
class QSortFilterProxyModel;
class QItemSelection;
class QMAMSGroupEmployeeModel;
class QMAMSGroupEmployeeGroupModel;
class QMEmployeeGroupViewModel;

namespace Ui
{
    class QMAMSGroupSettingsWidget;
}

/// Settings widget for account management system.
/// \author Christian Kr, Copyright 2020
class QMAMSGroupSettingsWidget: public QMSettingsWidget
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object for the qt system.
    explicit QMAMSGroupSettingsWidget(QWidget *parent = nullptr);

    /// Destructor
    ~QMAMSGroupSettingsWidget() override;

    /// This slot has to be implemented by parent class. When calling this
    /// function, it should take a QSettings object or data Model
    /// (bspw. SqlModel) to save every changes to a certificate based system.
    void saveSettings() override;

    /// Override from QMSettingsWidget.
    void revertChanges() override;

    /// Override from QMSettingsWidget.
    void loadSettings() override;

public slots:
    /// Add a new group.
    void addGroup();

    /// Add a new access mode.
    void addAccessMode();

    /// Remove a group.
    void removeGroup();

    /// Remove an access mode.
    void removeAccessMode();

    /// Group selection changed.
    /// \param deselected Item that has been deselected.
    /// \param selected Item that has been selected.
    void groupSelectionChanged(const QModelIndex &selected, const QModelIndex &deselected);

    /// Access mode selection changed.
    /// \param deselected Item that has been deselected.
    /// \param selected Item that has been selected.
    void accessModeSelectionChanged(const QModelIndex &selected, const QModelIndex &deselected);

    /// Group access mode correlation table selection changed.
    /// \param deselected Item that has been deselected.
    /// \param selected Item that has been selected.
    void groupAccessModeSelectionChanged(const QModelIndex &selected, const QModelIndex &deselected);

    /// Employee selection changed.
    /// \param deselected Item that has been deselected.
    /// \param selected Item that has been selected.
    void employeeSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    /// Employee correlation table selection changed.
    /// \param deselected Item that has been deselected.
    /// \param selected Item that has been selected.
    void groupEmployeeSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    /// Employee group selection changed.
    /// \param deselected item that has been deselected
    /// \param selected item that has been selected
    void employeeGroupSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    /// Employee group correlation table selection changed.
    /// \param deselected item that has been deselected
    /// \param selected item that has been selected
    void groupEmployeeGroupSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    /// Change the name of the group.
    void changeName();

    /// Change the activated state of the group.
    void changeActiveState();

    /// Select all employee.
    void selectAllEmployee();

    /// Deselect all employee.
    void deselectAllEmployee();

    /// Select all group employee.
    void selectAllGroupEmployee();

    /// Deselect all group employee.
    void deselectAllGroupEmployee();

    /// Add an employee.
    void addEmployee();

    /// Remove the selected employees.
    void removeEmployee();

    /// Add an employee group.
    void addEmployeeGroup();

    /// Remove an employee group.
    void removeEmployeeGroup();

private:
    /// Update data data.
    void updateData();

    /// Run action for enabling the usergroup list.
    /// \param selRow Selected row.
    void activateGroup(int selRow);

    /// Run action for disabling the usergroup list.
    void deactivateGroup();

    /// Search for an access mode in group proxy data.
    /// \param name The name of the group to search for.
    /// \return True if minimum one exist, else false.
    bool groupAccessModeProxyContainsAccessMode(const QString &accessMode);

    /// Search for an employee name in the group employee proxy data.
    /// \param name The name of the employee to search for.
    /// \return True if found, else false.
    bool groupEmployeeProxyContainsEmployee(const QString &name);

    /// Search for an employee group name in the group employee group proxy data.
    /// \param name The name of the employee group to search for.
    /// \return True if found, else false.
    bool groupEmployeeGroupProxyContainsEmployeeGroup(const QString &name);

    /// Search for group name in group data.
    /// \param The group name to search for.
    /// \return True if found, else false.
    bool groupContainsGroupname(const QString &groupname);

    Ui::QMAMSGroupSettingsWidget *ui;

    std::unique_ptr<QMAMSAccessModeModel> amsAccessModeModel;
    std::unique_ptr<QMAMSGroupModel> amsGroupModel;
    std::unique_ptr<QMAMSGroupEmployeeModel> amsGroupEmployeeModel;
    std::unique_ptr<QMAMSGroupEmployeeGroupModel> amsGroupEmployeeGroupModel;
    std::unique_ptr<QMEmployeeModel> employeeModel;
    std::unique_ptr<QMEmployeeGroupViewModel> employeeGroupViewModel;
    std::unique_ptr<QMAMSGroupAccessModeModel> amsGroupAccessModeModel;

    std::unique_ptr<QSortFilterProxyModel> amsGroupAccessModeProxyModel;
    std::unique_ptr<QSortFilterProxyModel> amsGroupEmployeeProxyModel;
    std::unique_ptr<QSortFilterProxyModel> amsGroupEmployeeGroupProxyModel;
};

#endif // QMAMSGROUPSETTINGSWIDGET_H
