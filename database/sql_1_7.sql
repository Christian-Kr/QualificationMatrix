-- Employee: Rename shift column to group column

ALTER TABLE "Employee" RENAME COLUMN "shift" TO "group";

-- Shift: Rename to EmployeeGroup

ALTER TABLE "Shift" RENAME TO "EmployeeGroup";

-- EmployeeGroupView definition

DROP VIEW "ShiftView";

CREATE VIEW IF NOT EXISTS "EmployeeGroupView" AS
SELECT
    EmployeeGroup.id,
    EmployeeGroup.name
FROM
    EmployeeGroup;

-- Recreate EmployeeView

DROP VIEW "EmployeeView";

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
    Employee LEFT JOIN EmployeeGroup ON Employee."group" = EmployeeGroup.id
WHERE
    Employee.active = 1;

-- Update version of database

UPDATE "Info" SET "value" = "1" WHERE "name" = "MAJOR";
UPDATE "Info" SET "value" = "7" WHERE "name" = "MINOR";
