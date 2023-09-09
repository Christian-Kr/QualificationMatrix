// qmappcachemanager.h is part of QualificationMatrix
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

#ifndef QMAPPCACHEMANAGER_H
#define QMAPPCACHEMANAGER_H

#include <QObject>

#include <memory>

/// The struct is an information holder for appcache information. This includes file information
/// about the current application version, the files that need to be copied and so on. Such a meta
/// file exists in the local source - if any exists - and in the remote source, where the new
// version is placed.
struct QMAppCacheMetaInfo
{
    // todo: fill with config information
};

/// The app cache manager is responsible to handle app caches to the copy process and validate the
/// copy. It also responsible for the security of the process (checksums etc.).
/// \author Christian Kr, Copyright 2023
class QMAppCacheManager : public QObject
{
    Q_OBJECT

public:
    /// Get a singleton instance of the app cache manager.
    /// \return singleton instance of the app cache manager
    static QMAppCacheManager * getInstance()
    {
        if (instance == nullptr)
        {
            instance = new QMAppCacheManager();
        }

        return instance;
    }

    /// Delete the static variable.
    [[maybe_unused]] static void resetInstance()
    {
        if (instance != nullptr)
        {
            delete instance;
            instance = nullptr;
        }
    }

    /// Trigger process.
    bool trigger();

signals:

private:
    /// Constructor
    QMAppCacheManager();

    /// Destructor
    ~QMAppCacheManager() override;

    /// Read in the meta information from the app cache file in the root directory of every
    /// application.
    /// \param appCacheMetaInfo meta info object that holds the information
    static bool readMetaInfo(QMAppCacheMetaInfo &appCacheMetaInfo);

    /// Read in the settings of the app cache, that might be changed by the user.
    bool readSettings();

    // Variables
    static QMAppCacheManager *instance;

    std::unique_ptr<QMAppCacheMetaInfo> m_metaInfo;

    std::unique_ptr<QString> m_sourcePath;
    std::unique_ptr<QString> m_targetPath;
};

#endif // QMAPPCACHEMANAGER_H
