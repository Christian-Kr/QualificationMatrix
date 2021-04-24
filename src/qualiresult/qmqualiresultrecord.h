// qmqualiresultrecord.h is part of QualificationMatrix
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

#ifndef QMQUALIRESULTRECORD_H
#define QMQUALIRESULTRECORD_H

#include <QObject>

/// Result object for every record in qualiresultmodel.
/// \author Christian Kr, Copyright 2020
class QMQualiResultRecord: public QObject
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit QMQualiResultRecord(QObject *parent = nullptr);

    /// Destructor
    ~QMQualiResultRecord() override = default;

    void setFirstName(const QString &name) { firstName = name; }
    QString getFirstName() const { return firstName; }

    void setLastName(const QString &name) { lastName = name; }
    QString getLastName() const { return lastName; }

    void setFunction(const QString &fun) { func = fun; }
    QString getFunction() const { return func; }

    void setTraining(const QString &training) { train = training; }
    QString getTraining() const { return train; }

    void setTrainingDataState(const QString &tDState) { trainDataState = tDState; }
    QString getTrainingDataState() const { return trainDataState; }

    void setVital(const bool vit) { vital = vit; }
    bool isVital() const { return vital; }

    void setNextDate(const QString &nDate) { nextDate = nDate; }
    QString getNextDate() const { return nextDate; }

    void setQualiState(const QString &qState) { qualiState = qState; }
    QString getQualiState() const { return qualiState; }

    void setLastDate(const QString &lDate) { lastDate = lDate; }
    QString getLastDate() const { return lastDate; }

    void setTrainingState(const QString &tState) { trainState = tState; }
    QString getTrainingState() const { return trainState; }

    void setInterval(const int interv) { this->interval = interv; }
    int getInterval() const { return interval; }

    void setInformation(const QString &info) { information = info; }
    QString getInformation() { return information; }

private:
    QString information;
    QString firstName;
    QString lastName;
    QString func;
    QString train;
    QString nextDate;
    QString qualiState;
    QString lastDate;
    QString trainState;
    QString trainDataState;
    int interval;

    bool vital;
};

#endif // QMQUALIRESULTRECORD_H
