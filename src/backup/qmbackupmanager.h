// qmbackupmanager.h is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.

#ifndef QMBACKUPMANAGER_H
#define QMBACKUPMANAGER_H

#include <QObject>

#include <memory>

class QFileInfo;
class QFile;
class QDateTime;

/// Enum for backup modes.
enum class BackupMode
{
    // The backup will be done every time the trigger will run. This is every time, when someone opens the application.
    EVERY_TRIGGER = 0,

    // The backup will be done only, when there is no backup for the given day.
    ONCE_A_DAY = 1,

    // The backup will done every time, when there is a change recognized. This might be tested bei different hash
    // values of the database files.
    ONLY_AFTER_CHANGE = 2
};

/// General information about the backup file. This one holds all general information that are hold in the general
/// section int the backup config file.
struct QMGeneralInformation
{
    qint64 version = 0;

    // The default mode is doing a backup on every trigger.
    BackupMode mode = BackupMode::EVERY_TRIGGER;

    // The maximum number of backups that should exist. More than the maximum number of backups will be deleted. The
    // number zero is the default setting and means an inifit number of backups.
    int maxBackups = 0;
};

/// The struct is an information holder for backup data. One struct object is responsible for one backup entry.
struct QMBackupMetaInfo
{
    /// Constructor
    QMBackupMetaInfo();

    /// Copy constructor
    /// \param other The object to copy.
    QMBackupMetaInfo(const QMBackupMetaInfo &other) noexcept;

    /// Assigment operator
    /// \param other The object to assign.
    /// \return The new object.
    QMBackupMetaInfo& operator=(const QMBackupMetaInfo &other);

    /// Compare operator
    /// \param other The object to compare against.
    /// \return True if this object is less than other, else false.
    bool operator<(const QMBackupMetaInfo &other) const;

    // The file of the backup which holds the meta information.
    std::unique_ptr<QFile> metaFile;

    // Integrity check will create hash values of the original and the copied file after dopy process and compare them.
    // Since the first check, every further check just compares the new hash value to the existing one in
    // integrityHash.
    bool integrityCheckPassed = false;
    std::unique_ptr<QString> integrityHash;

    // The file path of the backup file starting at the backup folder level.
    std::unique_ptr<QString> relFilePath;

    // The date when the backup has been created.
    std::unique_ptr<QDateTime> creationDate;
};

/// The backup manager. It can handle local file to be backuped into a system of backups with meta information. There
/// are two different files to handle. The settings file, which will only be readable. Writing can only be done
/// manually by opening the file. The meta information files will be placed next to the backup file and there is always
/// file for every backup with the same name, but different extension.
/// \author Christian Kr, Copyright 2022
class QMBackupManager : public QObject
{
    Q_OBJECT

public:
    /// Get a singleton instance of the backup manager.
    /// \return Singleton instance of the backup manager.
    static QMBackupManager * getInstance()
    {
        if (instance == nullptr)
        {
            instance = new QMBackupManager();
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

    /// Trigger backup. Thereby apply rules for running a backup, cause triggered is not the same as running. The
    /// system might be triggered regulary to check whether a backup should be running or not. For example if a backup
    /// should only be done once a day and not every time while opening the application, the backup system must be
    /// triggered every time the applications opened, but there won't be a backup done every time.
    ///
    /// The function will check, if the system needs a reinitialization cause of changes to the config file.
    ///
    /// \return True if success, else false.
    bool trigger();

    /// Get a list of all backup meta information files.
    /// \return List of all meta info files.
    QList<QMBackupMetaInfo> getBackupMetaInfoList();

signals:

private:
    /// Constructor
    QMBackupManager();

    /// Destructor
    ~QMBackupManager() override;

    /// This will run a backup, no matter if any rule fits.
    ///
    /// The function will check, if the system needs a reinitialization cause of changes to the config file.
    ///
    /// \return True if success, else false.
    bool run();

    /// Read in all settings for the backup system. The settings are stored in the application settings object and
    /// should be defined as centralized settings to make it consistent with all shared user.
    /// \return False if something fails, else true.
    bool readSettings();

    /// Read in all information about a given backup file. The result will be written to the given backupMetaInfo
    /// object. The file member should have an existing file already set.
    /// \return False if something fails, else true.
    static bool readBackupMetaInfo(QMBackupMetaInfo &backupMetaInfo);

    /// Write given backup meta information into a meta info file. The metaFile struct variable should be set before
    /// calling the function.
    /// \param backupMetaInfo The meta information to save.
    /// \return False if something fails, else true.
    static bool writeBackupMetaInfo(QMBackupMetaInfo &backupMetaInfo);

    /// Initialize the database file location.
    /// \return False if something fails, else true.
    bool initializeDatabaseFileLocation();

    // Variables
    static QMBackupManager *instance;

    std::unique_ptr<QMGeneralInformation> m_generalInformation;

    std::unique_ptr<QString> m_backupFolderPath;
    std::unique_ptr<QFileInfo> m_databaseFileInfo;
};

#endif // QMBACKUPMANAGER_H
