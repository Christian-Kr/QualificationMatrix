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

// Forward declaration
class QProgressDialog;

/// Enum object for multiple error codes while running the app cache manager.
enum class AppCacheError
{
    NONE,
    SOURCE_NOT_EXIST,
    SOURCE_NOT_DIR,
    TARGET_NOT_EXIST,
    TARGET_NOT_DIR,
    META_NOT_EXIST,
    META_NOT_READABLE,
    META_PARSE_FAILED,
    COPY_FAILED
};

// Holds information about a file that should be copied as a part of the app cache system.
struct QMAppCacheMetaInfoFile
{
    // File name with extension, but without its path.
    QString name;

    // The path to the folder, where the file is stored.
    QString folderPath;

    // The calculated hash value of the file. The hash value will be created with sha512.
    QString hash;
};

/// The struct is an information holder for appcache information. This includes file information
/// about the current application version, the files that need to be copied and so on. Such a meta
/// file exists in the local source - if any exists - and in the remote source, where the new
/// version is placed.
struct QMAppCacheMetaInfo
{
    // version number decides, when to update the target with the source
    int versionMajor = 0;
    int versionMinor = 0;

    // file list
    QList<QMAppCacheMetaInfoFile> files;
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

    /// Get the last error type.
    /// \return the error type of the last error
    [[nodiscard]] AppCacheError lastErrorType() const { return m_lastErrorType; }

    /// Get the last error message.
    /// \return the error message of the last error
    [[nodiscard]] QString lastErrorMessage() const { return m_lastErrorText; }

signals:

private:
    /// Constructor
    QMAppCacheManager();

    /// Destructor
    ~QMAppCacheManager() override;

    /// Read in the meta information from the app cache file in the root directory of every
    /// application.
    /// \param metaInfo meta info object to write parsed information to
    /// \param path where the app cache meta file can be found
    /// \return true if reading succeed, else false
    bool readMetaInfo(std::unique_ptr<QMAppCacheMetaInfo> &metaInfo, const QString &path);

    /// Parse meta info data from json.
    /// \param metaInfo meta info object to write parsed information to
    /// \param data to be parsed
    /// \return true if reading succeed, else false
    bool parseMetaInfo(std::unique_ptr<QMAppCacheMetaInfo> &metaInfo, const QByteArray &data);

    /// Parse the version part of the config.
    /// \param metaInf meta info object to write parsed information to
    /// \param jsonObject the json object to read the version object from
    bool parseVersionFromMetaInfo(std::unique_ptr<QMAppCacheMetaInfo> &metaInfo,
            QJsonObject &jsonObject);

    /// Parse the files part of the config.
    /// \param metaInf meta info object to write parsed information to
    /// \param jsonObject the json object to read the files object from
    bool parseFilesFromMetaInfo(std::unique_ptr<QMAppCacheMetaInfo> &metaInfo,
            QJsonObject &jsonObject);

    /// Read in the settings of the app cache, that might be changed by the user.
    bool readSettings();

    /// Copy all files from source to target.
    /// \return true if succeed, else false
    bool copyFiles();

    // Variables
    static QMAppCacheManager *instance;

    std::unique_ptr<QMAppCacheMetaInfo> m_metaInfoSource;
    std::unique_ptr<QMAppCacheMetaInfo> m_metaInfoTarget;

    QString m_sourcePath;
    QString m_targetPath;

    AppCacheError m_lastErrorType;
    QString m_lastErrorText;

    std::unique_ptr<QProgressDialog> m_progressDialog;
};

#endif // QMAPPCACHEMANAGER_H
