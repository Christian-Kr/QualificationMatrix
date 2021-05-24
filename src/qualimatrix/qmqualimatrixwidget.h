// qmqualimatrixwidget.h is part of QualificationMatrix
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

#ifndef QMQUALIMATRIXWIDGET_H
#define QMQUALIMATRIXWIDGET_H

#include "framework/qmwinmodewidget.h"

#include <memory>

// Forward declaration
class QMQualiMatrixModel;
class QSortFilterProxyModel;
class QSqlTableModel;
class QSqlRelationalTableModel;
class QMenu;

namespace Ui
{
class QMQualiMatrixWidget;
}

/// The view of the quali matrix.
/// \author Christian Kr, Copyright 2020
class QMQualiMatrixWidget: public QMWinModeWidget
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent widget.
    explicit QMQualiMatrixWidget(QWidget *parent = nullptr);

    /// Destructor
    ~QMQualiMatrixWidget() override;

    /// Load settings
    void loadSettings();

    /// Save settings
    void saveSettings();

public slots:
    // Actions after the model cache has been build.
    void afterBuildCacheModel();

    /// Sort the quali matrix by primary key.
    void trainSortPrimaryKey();

    /// Sort the quali matrix by name.
    void trainSortName();

    /// Sort the quali matrix by group.
    void trainSortGroup();

    /// Sort order ascending.
    void trainSortAscending();

    /// Sort order descending();
    void trainSortDescending();

    /// Show a context menu for horizontal header.
    void customHorizontalContextMenuRequest(QPoint pos);

    /// Sort the quali matrix by primary key.
    void funcSortPrimaryKey();

    /// Sort the quali matrix by name.
    void funcSortName();

    /// Sort the quali matrix by group.
    void funcSortGroup();

    /// Sort order ascending.
    void funcSortAscending();

    /// Sort order descending();
    void funcSortDescending();

    /// Show a context menu for horizontal header.
    void customVerticalContextMenuRequest(QPoint pos);

    /// The visibility state of the filter widget has changed.
    void filterVisibilityChanged();

    /// Change the filter widget visibility.
    void switchFilterVisibility();

    /// \brief update by loading data from datamanager
    void updateData();

    void updateFilter();

    void resetFilter();

    void updateHeaderCache();

    void updateColors();

    void switchLockMode();

    void enableLocked();

    /// Gets called when the selected cell changes.
    /// \param current
    /// \param previous
    void selectionChanged(const QModelIndex & current, const QModelIndex & previous);

    /// Extend the disable locked mode.
    void extendDisableLocked();

    /// Run extended selection for training group.
    void extSelTrainGroup();

    /// Run extended selection for function group.
    void extSelFuncGroup();

    /// Run extennded selection for functions.
    void extSelFunc();

    /// Run extended selection for trainings.
    void extSelTrain();

    /// Show info dialog with given header index.
    /// \param logicalIndex Logical header index.
    void showTrainSectionHeaderInfo(int logicalIndex);

    /// Close the train info widget.
    void closeTrainDetails();

    /// Reset train filter.
    void resetTrain();

    /// Reset func filter.
    void resetFunc();

    /// Reset train group filter.
    void resetTrainGroup();

    /// Reset func group filter.
    void resetFuncGroup();

    /// Update the model and view.
    void updateModel();

private:
    /// Build context menus.
    void buildContextMenus();

    Ui::QMQualiMatrixWidget *ui;

    std::unique_ptr<QMQualiMatrixModel> qualiMatrixModel;
    std::unique_ptr<QSqlTableModel> funcViewModel;
    std::unique_ptr<QSqlTableModel> trainViewModel;
    std::unique_ptr<QSqlTableModel> funcGroupViewModel;
    std::unique_ptr<QSqlTableModel> trainGroupViewModel;

    QSortFilterProxyModel *qualiMatrixFuncFilterModel;

    QTimer *lockModeTimer;

    QMenu *contextHorizontalHeader;
    QMenu *contextVerticalHeader;
};

#endif // QMQUALIMATRIXWIDGET_H
