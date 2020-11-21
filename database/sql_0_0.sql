-- TrainDataState definition

CREATE TABLE "TrainDataState" (
	"id"	INTEGER,
	"name"	TEXT,
	PRIMARY KEY("id")
);

-- Shift definition

CREATE TABLE "Shift" (
	"id"	INTEGER,
	"name"	TEXT NOT NULL,
	PRIMARY KEY("id")
);

-- QualiState definition

CREATE TABLE "QualiState" (
	"id"	INTEGER,
	"name"	TEXT,
	PRIMARY KEY("id")
);

-- Info definition

CREATE TABLE "Info" (
	"id"	INTEGER,
	"name"	TEXT,
	"value"	TEXT,
	PRIMARY KEY("id")
);

INSERT INTO "Info" ("name", "value") VALUES("MAJOR", "0");
INSERT INTO "Info" ("name", "value") VALUES("MINOR", "0");

-- FuncGroup definition

CREATE TABLE "FuncGroup" (
	"id"	INTEGER,
	"name"	TEXT,
	"color"	TEXT,
	PRIMARY KEY("id")
);

-- TrainGroup definition

CREATE TABLE "TrainGroup" (
	"id"	INTEGER,
	"name"	TEXT,
	"color"	TEXT,
	PRIMARY KEY("id")
);

-- Employee definition

CREATE TABLE "Employee" (
	"id"	INTEGER,
	"name"	TEXT NOT NULL,
	"shift"	INTEGER NOT NULL,
	PRIMARY KEY("id"),
	FOREIGN KEY("shift") REFERENCES `Shift`("id")
);

-- Train definition

CREATE TABLE "Train" (
	"id"	INTEGER,
	"name"	TEXT,
	"group"	INTEGER,
	"interval"	INTEGER,
	"legaly_necessary" INTEGER,
	PRIMARY KEY("id"),
	FOREIGN KEY("group") REFERENCES "TrainGroup"("id")
);

-- TrainData definition

CREATE TABLE "TrainData" (
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

CREATE TABLE `TrainException` (
	"id"	INTEGER,
	"employee"	INTEGER,
	"train"	INTEGER,
	"explanation"	TEXT,
	FOREIGN KEY("train") REFERENCES "Train"("id"),
	FOREIGN KEY("employee") REFERENCES "Employee"("id"),
	PRIMARY KEY("id")
);

-- Func definition

CREATE TABLE "Func" (
	"id"	INTEGER,
	"name"	TEXT,
	"group"	INTEGER,
	FOREIGN KEY("group") REFERENCES "FuncGroup"("id"),
	PRIMARY KEY("id")
);

-- EmployeeFunc definition

CREATE TABLE "EmployeeFunc" (
	"id"	INTEGER,
	"employee"	INTEGER,
	"func"	INTEGER,
	PRIMARY KEY("id"),
	FOREIGN KEY("employee") REFERENCES "Employee"("id"),
	FOREIGN KEY("func") REFERENCES "Func"("id")
);

-- QualiData definition

CREATE TABLE "QualiData" (
	"id"	INTEGER,
	"func"	INTEGER,
	"train"	INTEGER,
	"qualistate"	INTEGER,
	PRIMARY KEY("id"),
	FOREIGN KEY("train") REFERENCES "Train"("id"),
	FOREIGN KEY("func") REFERENCES "Func"("id"),
	FOREIGN KEY("qualistate") REFERENCES "QualiState"("id")
);
