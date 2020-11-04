//
// qualiresultrecord.h is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with QualificationMatrix.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef QUALIRESULTRECORD_H
#define QUALIRESULTRECORD_H

#include <QObject>

/**
 * @brief result object for every record in qualiresultmodel
 * @author Christian Kr, Copyright (c) 2020
 */
class QualiResultRecord: public QObject
{
Q_OBJECT

public:
    /**
     * @brief constructor
     * @param parent
     */
    explicit QualiResultRecord(QObject *parent = nullptr);

    ~QualiResultRecord();

    void setFirstName(const QString &firstName)
    { this->firstName = firstName; }

    QString getFirstName() const
    { return firstName; }

    void setLastName(const QString &lastName)
    { this->lastName = lastName; }

    QString getLastName() const
    { return lastName; }

    void setFunction(const QString &func)
    { this->func = func; }

    QString getFunction() const
    { return func; }

    void setTraining(const QString &train)
    { this->train = train; }

    QString getTraining() const
    { return train; }

    void setTrainingDataState(const QString &trainDataState)
    {
        this->trainDataState = trainDataState;
    }

    QString getTrainingDataState() const
    { return trainDataState; }

    void setVital(const bool vital)
    { this->vital = vital; }

    bool isVital() const
    { return vital; }

    void setNextDate(const QString &nextDate)
    { this->nextDate = nextDate; }

    QString getNextDate() const
    { return nextDate; }

    void setQualiState(const QString &qualiState)
    { this->qualiState = qualiState; }

    QString getQualiState() const
    { return qualiState; }

    void setLastDate(const QString &lastDate)
    { this->lastDate = lastDate; }

    QString getLastDate() const
    { return lastDate; }

    void setTrainingState(const QString &trainState)
    { this->trainState = trainState; }

    QString getTrainingState() const
    { return trainState; }

private:
    QString firstName;
    QString lastName;
    QString func;
    QString train;
    QString nextDate;
    QString qualiState;
    QString lastDate;
    QString trainState;
    QString trainDataState;

    bool vital;
};

#endif // QUALIRESULTRECORD_H
