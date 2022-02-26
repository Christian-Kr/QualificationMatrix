-- Employee is temporarily deactivated. This flag can be set und unset by everyone in contrast to the
-- active flag, which cannot be disabled inside the software.
ALTER TABLE "Employee" ADD COLUMN "temporarily_deactivated" INTEGER default 0;

-- Employee is a personnel leasing one, which might affect the trainings they need.
ALTER TABLE "Employee" ADD COLUMN "personnel_leasing" INTEGER default 0;

-- Employee is a practicant.
ALTER TABLE "Employee" ADD COLUMN "trainee" INTEGER default 0;

-- Employee is an apprentice.
ALTER TABLE "Employee" ADD COLUMN "apprentice" INTEGER default 0;

-- Create new employee view.
DROP VIEW IF EXISTS "EmployeeView";
CREATE VIEW IF NOT EXISTS "EmployeeView" AS
SELECT
    Employee.id,
    Employee.name,
    Employee.shift,
    Employee.temporarily_deactivated,
    Employee.personnel_leasing,
    Employee.trainee,
    Employee.apprentice
FROM
    Employee
        LEFT JOIN Shift ON Employee.shift = Shift.id
WHERE
    Employee.active = 1;

-- Update version of database

UPDATE "Info" SET "value" = "1" WHERE "name" = "MAJOR";
UPDATE "Info" SET "value" = "5" WHERE "name" = "MINOR";
