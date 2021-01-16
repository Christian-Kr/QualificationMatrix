-- Add option in info for file_location, which is necessary for certificates

INSERT INTO "Info" ("name", "value") VALUES("signinglist_image", "");

-- Change Employee to mark an employee as active or not

ALTER TABLE "Employee" ADD COLUMN "active" INTEGER default 1;

-- Update version of database

UPDATE "Info" SET "value" = "1" WHERE "name" = "MAJOR";
UPDATE "Info" SET "value" = "1" WHERE "name" = "MINOR";
