//
// qualimatrixwidget.h is part of QualificationMatrix
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

#ifndef QMQUALIMATRIXWIDGET_H
#define QMQUALIMATRIXWIDGET_H

#include <QWidget>
#include <memory>

// Forward declaration
class QualiMatrixModel;
class QSortFilterProxyModel;
class QSqlTableModel;
class QSqlRelationalTableModel;

namespace Ui
{
class QMQualiMatrixWidget;
}

/// The view of the quali matrix.
/// \author Christian Kr, Copyright 2020
class QMQualiMatrixWidget: public QWidget
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent widget.
    explicit QMQualiMatrixWidget(QWidget *parent = nullptr);

    /// Destructor
    ~QMQualiMatrixWidget() override;

public slots:

    /// \brief update by loading data from datamanager
    void updateData();

    void updateFilter();

    void resetFilter();

    void updateHeaderCache();

    void updateColors();

    void switchLockMode();

    void enableLocked();

private:
    Ui::QMQualiMatrixWidget *ui;

    std::shared_ptr<QualiMatrixModel> qualiMatrixModel;
    std::shared_ptr<QSqlRelationalTableModel> funcModel;
    std::shared_ptr<QSqlRelationalTableModel> trainModel;
    std::shared_ptr<QSqlTableModel> funcGroupModel;
    std::shared_ptr<QSqlTableModel> trainGroupModel;

    QSortFilterProxyModel *qualiMatrixFuncFilterModel;

    QTimer *lockModeTimer;
};

#endif // QMQUALIMATRIXWIDGET_H
