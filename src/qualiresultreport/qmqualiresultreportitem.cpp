// qmqualiresultreportitem.cpp is part of QualificationMatrix
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

#include "qmqualiresultreportitem.h"

#include <QDebug>

QMQualiResultReportItem::QMQualiResultReportItem(QObject *parent)
    : QObject(parent)
    , trainId(0)
    , trainName(new QString())
    , trainResultTotal(0)
    , trainResultOk(0)
{}

QMQualiResultReportItem::~QMQualiResultReportItem()
{
    delete trainName;
}

QMQualiResultReportItem::QMQualiResultReportItem(const QMQualiResultReportItem &obj)
    : trainId(obj.trainId)
    , trainResultTotal(obj.trainResultTotal)
    , trainResultOk(obj.trainResultOk)
    , trainName(new QString())
{
    setTrainName(*obj.trainName);
}

QMQualiResultReportItem& QMQualiResultReportItem::operator=(const QMQualiResultReportItem &obj)
{
    trainId = obj.trainId;
    trainResultTotal = obj.trainResultTotal;
    trainResultOk = obj.trainResultOk;
    *trainName = *obj.trainName;

    return *this;
}
