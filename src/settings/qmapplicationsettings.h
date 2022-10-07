// qmapplicationsettings.h is part of QualificationMatrix
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

#ifndef QMAPPLICATIONSETTINGS_H
#define QMAPPLICATIONSETTINGS_H

#include "qmsettings.h"

class QSettings;

/// \brief Class for application wide settings on the certificate system.
///
/// This settings class implement the application wide settings with centralized feature. It is only
/// for certificate based settings and for application NOT database related settings.
///
/// (example: Color of qm grid is an application setting! Files on filesystem or in sql table blobb
/// is an database setting, cause the database structure depend on it! It behaves like data and
/// view separation in the mfc arch.)
///
/// The object of this class is a singleton, cause there is only one application to run and
/// therefore only one application wide settings.
///
/// The centralized setting can only be set in local. Cause this certificate always exist.
class QMApplicationSettings: public QMSettings
{
Q_OBJECT

public:
    /// \brief get the singleton instance
    /// \return singleton instance
    static QMApplicationSettings &getInstance()
    {
        static QMApplicationSettings instance;
        return instance;
    }

    // delete functions that should not be used, cause of singleton pattern
    // they should be deleted public for better error message if anything goes wrong
    QMApplicationSettings(QMApplicationSettings const &) = delete;

    void operator=(QMApplicationSettings const &) = delete;

    /// \brief override of QMSettings::read
    virtual QVariant read(const QString &name) override;

    virtual QVariant read(const QString &name, const QVariant &defValue) override;

    /// \brief override of QMSettings::write
    virtual void write(const QString &name, const QVariant &value) override;

    enum PRIORITY
    {
        LOCAL, CENTRAL
    };

private:
    explicit QMApplicationSettings(QObject *parent = nullptr);

    ~QMApplicationSettings() override;

    void readCentralized();

    void initCentralized();

    void initLocal();

    bool centralizedAvailable;
    QString centralizedPath;
    QMApplicationSettings::PRIORITY priority;

    QSettings *centralized;
    QSettings *local;
};

#endif // QMAPPLICATIONSETTINGS_H
