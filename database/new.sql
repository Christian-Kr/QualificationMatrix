-- This script can be used to create a new database with all nedded tables. This script should be up to date with the
-- latest database version and the latest update script, that comes with the application.

-- TrainDataState definition

CREATE TABLE IF NOT EXISTS "TrainDataState" (
	"id"	INTEGER,
	"name"	TEXT,
    "color" TEXT,
	PRIMARY KEY("id")
);

-- EmployeeGroup definition

CREATE TABLE IF NOT EXISTS "EmployeeGroup" (
	"id"	INTEGER,
	"name"	TEXT NOT NULL,
	PRIMARY KEY("id")
);

-- QualiState definition

CREATE TABLE IF NOT EXISTS "QualiState" (
	"id"	INTEGER,
	"name"	TEXT,
	PRIMARY KEY("id")
);

INSERT INTO "QualiState" ("name") VALUES("Pflicht");
INSERT INTO "QualiState" ("name") VALUES("Angebot");
INSERT INTO "QualiState" ("name") VALUES("Sonstiges");

-- Info definition

CREATE TABLE IF NOT EXISTS "Info" (
	"id"	INTEGER,
	"name"	TEXT,
	"value"	TEXT,
	PRIMARY KEY("id")
);

-- TODO: Update
INSERT INTO "Info" ("name", "value") VALUES("MAJOR", "1");
INSERT INTO "Info" ("name", "value") VALUES("MINOR", "9");

-- FuncGroup definition

CREATE TABLE IF NOT EXISTS "FuncGroup" (
	"id"	INTEGER,
	"name"	TEXT,
	"color"	TEXT,
	PRIMARY KEY("id")
);

-- TrainGroup definition

CREATE TABLE IF NOT EXISTS "TrainGroup" (
	"id"	INTEGER,
	"name"	TEXT,
	"color"	TEXT,
    "qmresult_ignore" INTEGER default 0,
    "ignore_reason" TEXT,
	PRIMARY KEY("id")
);

-- Employee definition

CREATE TABLE IF NOT EXISTS "Employee" (
	"id"	INTEGER,
	"name"	TEXT NOT NULL,
	"group"	INTEGER NOT NULL,
    "active" INTEGER default 1,
    "temporarily_deactivated" INTEGER default 0,
    "personnel_leasing" INTEGER default 0,
    "trainee" INTEGER default 0,
    "apprentice" INTEGER default 0,
	PRIMARY KEY("id"),
	FOREIGN KEY("group") REFERENCES `EmployeeGroup`("id")
);

-- Train definition

CREATE TABLE IF NOT EXISTS "Train" (
	"id"	INTEGER,
	"name"	TEXT,
	"group"	INTEGER,
	"interval"	INTEGER,
    "legally_necessary" INTEGER,
    "content_description" TEXT,
	PRIMARY KEY("id"),
	FOREIGN KEY("group") REFERENCES "TrainGroup"("id")
);

-- TrainData definition

CREATE TABLE IF NOT EXISTS "TrainData" (
	"id"	INTEGER,
	"employee"	INTEGER,
	"train"	INTEGER,
	"date"	TEXT,
	"state"	INTEGER,
	PRIMARY KEY("id"),
	FOREIGN KEY("employee") REFERENCES "Employee"("id"),
	FOREIGN KEY("state") REFERENCES "TrainDataState"("id"),
	FOREIGN KEY("train") REFERENCES "Train"("id")
);

-- TrainException definition

CREATE TABLE IF NOT EXISTS `TrainException` (
	"id"	INTEGER,
	"employee"	INTEGER,
	"train"	INTEGER,
	"explanation"	TEXT,
	FOREIGN KEY("train") REFERENCES "Train"("id"),
	FOREIGN KEY("employee") REFERENCES "Employee"("id"),
	PRIMARY KEY("id")
);

-- Func definition

CREATE TABLE IF NOT EXISTS "Func" (
	"id"	INTEGER,
	"name"	TEXT,
	"group"	INTEGER,
	FOREIGN KEY("group") REFERENCES "FuncGroup"("id"),
	PRIMARY KEY("id")
);

-- EmployeeFunc definition

CREATE TABLE IF NOT EXISTS "EmployeeFunc" (
	"id"	INTEGER,
	"employee"	INTEGER,
	"func"	INTEGER,
	PRIMARY KEY("id"),
	FOREIGN KEY("employee") REFERENCES "Employee"("id"),
	FOREIGN KEY("func") REFERENCES "Func"("id")
);

-- QualiData definition

CREATE TABLE IF NOT EXISTS "QualiData" (
	"id"	INTEGER,
	"func"	INTEGER,
	"train"	INTEGER,
	"qualistate"	INTEGER,
	PRIMARY KEY("id"),
	FOREIGN KEY("train") REFERENCES "Train"("id"),
	FOREIGN KEY("func") REFERENCES "Func"("id"),
	FOREIGN KEY("qualistate") REFERENCES "QualiState"("id")
);

-- Info definition

CREATE TABLE IF NOT EXISTS "Info" (
    "id"        INTEGER,
    "name"      TEXT,
    "value"     TEXT,
    PRIMARY KEY("id")
);

-- Add option in info for file_location, which is necessary for certificates

INSERT INTO "Info" ("name", "value") VALUES("certificate_location", "external");
INSERT INTO "Info" ("name", "value") VALUES("certificate_location_path", "");

-- Certificate definition

CREATE TABLE IF NOT EXISTS "Certificate" (
    "id"            INTEGER,
    "name"          TEXT,
    "type"          TEXT,
    "path"          TEXT,
    "binary"        BLOB,
    "md5_hash"      TEXT,
    "add_date"      TEXT,
    "train_date"    TEXT,
    PRIMARY KEY("id")
);

-- TraiDataCertificate links TrainData and File extries

CREATE TABLE IF NOT EXISTS "TrainDataCertificate" (
    "id"            INTEGER,
    "train_data"    INTEGER,
    "certificate"   INTEGER,
    PRIMARY KEY("id"),
    FOREIGN KEY("train_data")       REFERENCES "TrainData"("id"),
    FOREIGN KEY("certificate")      REFERENCES "Certificate"("id")
);

-- Create view to make handling more easy.

CREATE VIEW IF NOT EXISTS TrainDataCertificateView AS
	SELECT
       TrainDataCertificate.id,
       TrainDataCertificate.train_data,
       TrainDataCertificate.certificate,
       Certificate.name,
       Certificate.md5_hash
	FROM
        TrainDataCertificate
    LEFT JOIN Certificate ON TrainDataCertificate.certificate = Certificate.id;

-- Create view for active employees

CREATE VIEW IF NOT EXISTS "EmployeeView" AS
SELECT
    Employee.id,
    Employee.name,
    Employee."group",
    Employee.temporarily_deactivated,
    Employee.personnel_leasing,
    Employee.trainee,
    Employee.apprentice
FROM
    Employee
        LEFT JOIN EmployeeGroup ON Employee."group" = EmployeeGroup.id
WHERE
    Employee.active = 1;

-- Create view for functions

CREATE VIEW IF NOT EXISTS FuncView AS
SELECT
    Func.id,
    Func.name,
    FuncGroup.name
FROM
    Func
LEFT JOIN FuncGroup ON
    Func."group" = FuncGroup.id;

-- Create view for function groups

CREATE VIEW IF NOT EXISTS FuncGroupView AS
SELECT
    FuncGroup.id,
    FuncGroup.name,
    FuncGroup.color
FROM
    FuncGroup;

-- Create view for trainings

CREATE VIEW IF NOT EXISTS TrainView AS
SELECT
    Train.id,
    Train.name,
    TrainGroup.name,
    Train.interval,
    Train.legally_necessary,
    Train.content_description
FROM
    Train
LEFT JOIN TrainGroup ON Train."group" = TrainGroup.id;

-- Create view for training groups

CREATE VIEW IF NOT EXISTS TrainGroupView AS
SELECT
    TrainGroup.id,
    TrainGroup.name,
    TrainGroup.color
FROM
    TrainGroup;

-- Create train data state view

CREATE VIEW IF NOT EXISTS TrainDataStateView AS
SELECT
    TrainDataState.id,
    TrainDataState.name
FROM
    TrainDataState;

-- Create employee group view

CREATE VIEW IF NOT EXISTS "EmployeeGroupView" AS
SELECT
    EmployeeGroup.id,
    EmployeeGroup.name
FROM
    EmployeeGroup;

-- Create train exception view

CREATE VIEW IF NOT EXISTS TrainExceptionView AS
SELECT
    TrainException.id,
    Employee.name,
    Train.name,
    TrainException.explanation
FROM
    TrainException
LEFT JOIN Employee ON TrainException.employee = Employee.id
LEFT JOIN Train ON TrainException.train = Train.id;

-- Create a quali data view

CREATE VIEW IF NOT EXISTS QualiDataView AS
SELECT
    QualiData.id,
    Func.name,
    Train.name,
    QualiState.name
FROM
    QualiData
LEFT JOIN Func ON QualiData.func = Func.id
LEFT JOIN Train ON QualiData.train = Train.id
LEFT JOIN QualiState ON QualiData.qualistate = QualiState.id;

-- Create employee func view data

CREATE VIEW IF NOT EXISTS EmployeeFuncView AS
SELECT
    EmployeeFunc.id,
    Employee.name,
    Func.name
FROM
    EmployeeFunc
LEFT JOIN Employee ON EmployeeFunc.employee = Employee.id
LEFT JOIN Func ON EmployeeFunc.func = Func.id;

-- Create train data view

CREATE VIEW IF NOT EXISTS TrainDataView AS
SELECT
    TrainData.id,
    Employee.name,
    Train.name,
    TrainData.date,
    TrainDataState.name
FROM
    TrainData
LEFT JOIN Employee ON TrainData.employee = Employee.id
LEFT JOIN Train ON TrainData.train = Train.id
LEFT JOIN TrainDataState ON TrainData.state = TrainDataState.id;

-- User for Access Management System

CREATE TABLE IF NOT EXISTS "AMSUser" (
    "amsuser_id"	                    INTEGER,
    "amsuser_name"	                    TEXT,
    "amsuser_username"                  TEXT,
    "amsuser_password"                  TEXT,
    "amsuser_last_login"                TEXT,
    "amsuser_unsuccess_login_num"       INTEGER DEFAULT 0,
    "amsuser_active"                    INTEGER DEFAULT 0,
    "amsuser_admin"                     INTEGER DEFAULT 0,
    PRIMARY KEY("amsuser_id")
);

INSERT INTO "AMSUser"
    ("amsuser_name", "amsuser_username", "amsuser_password", "amsuser_active", "amsuser_admin")
VALUES
    ("administrator", "administrator",
     "$argon2id$v=19$m=8192,t=100,p=1$w93g/9FGO2nAajwoi02FHw$tHjzDkGoscBHLJOGJoqh699KXvWf+JqmARrR2L4yzxk",
     1, 1);

-- Group for Access Management System

CREATE TABLE IF NOT EXISTS "AMSGroup" (
   "amsgroup_id"	                    INTEGER,
   "amsgroup_name"	                    TEXT,
   "amsgroup_active"                    INTEGER DEFAULT 0,
   PRIMARY KEY("amsgroup_id")
);

-- Connects the AMSUser with the AMSGroup

CREATE TABLE IF NOT EXISTS "AMSUserGroup" (
    "amsusergroup_id"                   INTEGER,
    "amsusergroup_user"                 INTEGER,
    "amsusergroup_group"                INTEGER,
    PRIMARY KEY("amsusergroup_id"),
    FOREIGN KEY("amsusergroup_user")    REFERENCES "AMSUser"("amsuser_id"),
    FOREIGN KEY("amsusergroup_group")   REFERENCES "AMSGroup"("amsgroup_id")
);

-- Modes of access for group

CREATE TABLE IF NOT EXISTS "AMSAccessMode" (
    "amsaccessmode_id"                  INTEGER,
    "amsaccessmode_name"                TEXT,
    "amsaccessmode_value"               INTEGER,
    "amsaccessmode_description"         TEXT,
    PRIMARY KEY("amsaccessmode_id")
);

-- Add default values for access mode
INSERT INTO "AMSAccessMode" ("amsaccessmode_name", "amsaccessmode_value", "amsaccessmode_description")
VALUES
       ("QM_MODE_READ", 0, "Read access to qualification matrix"),
       ("QM_MODE_WRITE", 1, "Write and read access to qualification matrix"),
       ("QR_MODE_READ", 2, "Read access to qualification result"),
       ("TD_MODE_READ", 3, "Read access to training data including certification management"),
       ("TD_MODE_WRITE", 4, "Write and read access to training data including certification management"),
       ("PER_DATA_CONFIG", 5, "Read and write access to the persistent tables in config");

-- Connects the acces modes with the groups

CREATE TABLE IF NOT EXISTS "AMSGroupAccessMode" (
    "amsgroupaccessmode_id"                         INTEGER,
    "amsgroupaccessmode_group"                      INTEGER,
    "amsgroupaccessmode_access_mode"                INTEGER,
    PRIMARY KEY("amsgroupaccessmode_id"),
    FOREIGN KEY("amsgroupaccessmode_group")         REFERENCES
        "AMSGroup" ("amsgroup_id"),
    FOREIGN KEY("amsgroupaccessmode_access_mode")   REFERENCES
        "AMSAccessMode"("amsaccessmode_id")
);

-- Connects employees with the groups

CREATE TABLE IF NOT EXISTS "AMSGroupEmployee" (
    "amsgroupemployee_id"                           INTEGER,
    "amsgroupemployee_group"                        INTEGER,
    "amsgroupemployee_employee"                     INTEGER,
    PRIMARY KEY("amsgroupemployee_id"),
    FOREIGN KEY("amsgroupemployee_group")           REFERENCES
        "AMSGroup" ("amsgroup_id"),
    FOREIGN KEY("amsgroupemployee_employee")        REFERENCES
        "Employee" ("id")
);

-- General information for access management system

CREATE TABLE IF NOT EXISTS "AMSGeneral" (
    "amsgeneral_id"            INTEGER,
    "amsgeneral_name"          TEXT,
    "amsgeneral_value"         TEXT,
    "amsgeneral_description"   TEXT,
    PRIMARY KEY("amsgeneral_id")
);


-- Create a new view for user access

CREATE VIEW AMSUserEmployeeView AS
SELECT DISTINCT
    AMSUser.amsuser_id as user_id,
    Employee.id as employee_id
FROM
    AMSUser, AMSGroup, AMSUserGroup, AMSGroupEmployee, Employee
LEFT OUTER JOIN AMSUserGroup AMSUserGroup1 ON
    AMSUser.amsuser_id = AMSUserGroup1.amsusergroup_user AND
    AMSGroup.amsgroup_id = AMSUserGroup1.amsusergroup_group
LEFT OUTER JOIN AMSGroupEmployee AMSGroupEmployee1 ON
    AMSGroup.amsgroup_id = AMSGroupEmployee1.amsgroupemployee_group AND
    Employee.id = AMSGroupEmployee1.amsgroupemployee_employee
WHERE
    AMSGroup.amsgroup_id = AMSUserGroup.amsusergroup_group AND
    AMSUser.amsuser_id = AMSUserGroup.amsusergroup_user AND
    AMSGroup.amsgroup_id = AMSGroupEmployee.amsgroupemployee_group AND
    Employee.id = AMSGroupEmployee.amsgroupemployee_employee;


-- Create table for ams groups and employee groups relation

CREATE TABLE IF NOT EXISTS "AMSGroupEmployeeGroup" (
    "amsgroupemployeegroup_id"                           INTEGER,
    "amsgroupemployeegroup_group"                        INTEGER,
    "amsgroupemployeegroup_employeegroup"                INTEGER,
    PRIMARY KEY("amsgroupemployeegroup_id"),
    FOREIGN KEY("amsgroupemployeegroup_group")           REFERENCES
        "AMSGroup" ("amsgroup_id"),
    FOREIGN KEY("amsgroupemployeegroup_employeegroup")   REFERENCES
        "EmployeeGroup" ("id")
);


-- Create a new view for user access with the help of employee groups

CREATE VIEW AMSUserEmployeeGroupView AS
SELECT DISTINCT
    AMSUser.amsuser_id as user_id,
    Employee.id as employee_id
FROM
    AMSUser, AMSGroup, AMSUserGroup, AMSGroupEmployeeGroup, EmployeeGroup, Employee
WHERE
    AMSUser.amsuser_id = AMSUserGroup.amsusergroup_user AND
    AMSGroup.amsgroup_id = AMSUserGroup.amsusergroup_group AND
    AMSGroup.amsgroup_id = AMSGroupEmployeeGroup.amsgroupemployeegroup_group AND
    EmployeeGroup.id = AMSGroupEmployeeGroup.amsgroupemployeegroup_employeegroup AND
    Employee.'group' = EmployeeGroup.id;

