-- AMSUser: Every user for an account can be an admin.

ALTER TABLE "AMSUser" ADD COLUMN "admin" INTEGER default 0;

-- Update version of database

UPDATE "Info" SET "value" = "1" WHERE "name" = "MAJOR";
UPDATE "Info" SET "value" = "6" WHERE "name" = "MINOR";
