// qmqualiresultreportitem.h is part of QualificationMatrix
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

#ifndef QMQUALIRESULTREPORTITEM_H
#define QMQUALIRESULTREPORTITEM_H

#include <QObject>

#include <memory>

/// Object for holding the information about one result item.
/// \author Christian Kr, Copyright 2022
class QMQualiResultReportItem : public QObject
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object of the object.
    explicit QMQualiResultReportItem(QObject *parent = nullptr);

    /// Destructor
    ~QMQualiResultReportItem() override;

    /// Copy constructur
    QMQualiResultReportItem(const QMQualiResultReportItem &obj);

    /// Operator =
    QMQualiResultReportItem& operator=(const QMQualiResultReportItem &obj);

    // Set and get for multiple object variables.

    void setTrainId(int id) { trainId = id; }
    int getTrainId() { return trainId; }

    void setTrainName(QString name) { *trainName = name; }
    QString getTrainName() { return *trainName; }

    void setTrainResultTotal(int resultTotal) { trainResultTotal = resultTotal; }
    int getTrainResultTotal() { return trainResultTotal; }

    void setTrainResultOk(int resultOk) { trainResultOk = resultOk; }
    int getTrainResultOk() { return trainResultOk; }

private:
    int trainId;
    QString *trainName;

    int trainResultTotal;
    int trainResultOk;
};

#endif // QMQUALIRESULTREPORTITEM_H
