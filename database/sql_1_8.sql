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


-- Update version of database

UPDATE "Info" SET "value" = "1" WHERE "name" = "MAJOR";
UPDATE "Info" SET "value" = "8" WHERE "name" = "MINOR";
