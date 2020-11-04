//
// qmqualimatrixheaderview.h is part of QualificationMatrix
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

#ifndef QMQUALIMATRIXHEADERVIEW_H
#define QMQUALIMATRIXHEADERVIEW_H

#include <QHeaderView>

/// Derives the header to allow custom design of column header.
/// \author Christian Kr, Copyright (c) 2020
class QMQualiMatrixHeaderView: public QHeaderView
{
Q_OBJECT

public:
    /// Constructor
    /// \param orientation Defines the orientation of the header.
    /// \param parent The parent object for the qt system.
    explicit QMQualiMatrixHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

    /// Destructor
    ~QMQualiMatrixHeaderView() override;

    /// Override from QHeaderView.
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;

    /// Override from QHeaderView.
    QSize sizeHint() const override;

public slots:
    /// Get size from settings and informate listener.
    void updateSizeSettings();

    /// Update the cache list with groups and colors.
    void updateFunctionGroupColors();

    /// Update the cache list with groups and colors.
    void updateTrainingGroupColors();

    /// Update the cache for legally necessary information.
    void updateTrainLegallyNecessary();

private:
    int horSectionHeight;
    int vertSectionWidth;

    QHash<QString, QString> *funcGroupColorCache;
    QHash<QString, QString> *trainGroupColorCache;
    QHash<QString, bool> *trainLegallyNecessaryCache;
};

#endif // QMQUALIMATRIXHEADERVIEW_H
