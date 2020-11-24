//
// qmmainwindow.h is part of QualificationMatrix
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

#ifndef QMMAINWINDOW_H
#define QMMAINWINDOW_H

#include <QMainWindow>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class QMMainWindow;
}
QT_END_NAMESPACE

// Forward declarations
class QProgressDialog;
class QMQualiResultWidget;
class QMQualiMatrixWidget;
class QTranslator;
class QSqlDatabase;

/// Main windows class for this application.
/// \author Christian Kr, Copyright 2020
class QMMainWindow: public QMainWindow
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object, if there is one.
    explicit QMMainWindow(QWidget *parent = nullptr);

    /// Destructor
    ~QMMainWindow() override;

public slots:
    /// Show the dialog to create a signing list.
    void showCreateSigningList();

    /// Show the about dialog of this application.
    void showAbout();

    /// Show the about dialog of Qt.
    void showAboutQt();

    /// Show the central settings dialog of this application.
    void showSettings();

    /// Show the dialog for manipulate the traning data.
    void showTrainData();

    /// Show a progress dialog to informate about a working progress.
    /// \param title The title of the progress dialog.
    /// \param text The text to give the user an information about the work that will be done.
    /// \param minSteps The minimum steps of updates for the progress widget.
    /// \param maxSteps The maximum steps of updates for the progress widget.
    void showProgress(
        const QString &title, const QString &text, const int &minSteps, const int &maxSteps);

    /// Save all readed settings centralized on one place.
    void saveSettings();

    /// Informate before models will be initialized in datamanager (time expensive).
    /// \param maxSteps Number of models that will be initialized.
    void beforeInitializeModels(int maxSteps);

    /// Informate that all models have been initialized in data manager.
    void afterInitializeModels();

    /// Informate before the qualimatrix cache will be build (time expensive).
    /// \param maxSteps Maximum number of steps in progress.
    void beforeQualiMatrixBuildCache(int maxSteps);

    /// Informate before qualiresult calculation will be done (time expensive).
    /// \param maxSteps Maximum number of steps in progress.
    void beforeQualiResultCalculation(int maxSteps);

    /// Only call when progressdialog object has been created and dialog is visible.
    /// \param currentStep Needs be lower than maximum step.
    void updateProgress(int currentStep);

    /// Should only be called after progressdialog has been shown.
    void closeProgress();

    /// Close the database, if there is currently an open one.
    /// \return False if it cannot be close or don't exist, else true.
    bool closeDatabase();

    /// Run the auto backup, which includes multiple steps.
    /// TODO: Put this function into a separate class.
    /// \param fileName The certificate name of the new backup certificate.
    /// \return True if success, else false.
    bool runAutoBackup(const QString &fileName);

    /// Shows the database dialog to manage the database information.
    void manageDatabase();

    /// Manage database information without the database dialog.
    /// \return True if it could be load and open from settings, else false.
    bool manageDatabaseFromSettings();

    /// Create an empty database with initial emtpy structure.
    void createEmptyDatabase();

    /// Save a database backup.
    /// \param db The database to save.
    /// \return True if success, else false.
    bool saveSingleDatabaseBackup(const QSqlDatabase &db);

    /// Open dialog for managing the certificate files.
    void manageCertificate();

    /// Show dialog for certificate integrity check.
    void showCertificateIntegrityCheck();

protected:
    /// Override from QMainWindow. This function will be called on closeing the widget.
    /// \param event The close event object that will be created.
    void closeEvent(QCloseEvent *event) override;

private:
    /// Initialize the connection of Qt's signal/slot concept.
    void initConnections();

    /// Initialize database information.
    void initDatabaseSettings();

    /// Save QSqlDatabase information to QMApplicationSettings.
    static void saveDatabaseSettings();

    /// Load QSqlDatabase information from QMApplicationSettings. A new database object will be
    /// created. If one exist with the name, the database will be closed and the object removed.
    /// \param dbName The name of the db that should be created from settings.
    void loadDatabaseFromSettings(const QString &dbName = "default");

    /// Some initial actions after database has been successfully loaded.
    void initAfterDatabaseOpened();

    Ui::QMMainWindow *ui;

    std::unique_ptr<QProgressDialog> progressDialog;
    std::unique_ptr<QMQualiResultWidget> qualiResultWidget;
    std::unique_ptr<QMQualiMatrixWidget> qualiMatrixWidget;
};

#endif // QMMAINWINDOW_H
