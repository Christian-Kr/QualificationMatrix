-- Change Employee to mark an employee as active or not

ALTER TABLE "Employee" ADD COLUMN "active" INTEGER default 1;

-- Update version of database

UPDATE "Info" SET "value" = "1" WHERE "name" = "MAJOR";
UPDATE "Info" SET "value" = "1" WHERE "name" = "MINOR";
