-- Create view for active employees

CREATE VIEW IF NOT EXISTS EmployeeView AS
SELECT
    Employee.id,
    Employee.name,
    Employee.shift
FROM
    Employee
        LEFT JOIN Shift ON Employee.shift = Shift.id
WHERE
    Employee.active = 1;

-- Add column to training for content description

ALTER TABLE "Train" ADD COLUMN "content_description" TEXT;

-- Update version of database

UPDATE "Info" SET "value" = "1" WHERE "name" = "MAJOR";
UPDATE "Info" SET "value" = "2" WHERE "name" = "MINOR";
