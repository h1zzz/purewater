-- MIT License Copyright (c) 2021, h1zzz

CREATE TABLE `device` (
	`id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
	`host` VARCHAR(64) NOT NULL, -- IP address connected to C2
	`status` TINYINT NOT NULL, -- Agent status, 1 online and 0 offline
	`arch` VARCHAR(64) NOT NULL DEFAULT '', -- Device architecture information
	`os` VARCHAR(64) NOT NULL DEFAULT '', -- Operating system
	`version` VARCHAR(32) NOT NULL DEFAULT '', -- Agent version
	`hostname` VARCHAR(64) NOT NULL DEFAULT '', -- Device host name
	`time` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, -- Time to connect to C2
	PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;


CREATE TABLE `address` (
	`id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
	`device_id` INT NOT NULL,
	`name` VARCHAR(64) NOT NULL, -- Network card name
	`ip` VARCHAR(64) NOT NULL, -- IP address
	`mac` VARCHAR(32) NOT NULL, -- Mac address of the network card
	PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;


CREATE TABLE `task` (
	`id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
	`device_id` INT NOT NULL,
	`type` VARCHAR(32) NOT NULL, -- Task type
	`param` TEXT NOT NULL, -- Task parameters
	`status` TINYINT NOT NULL, -- Task execution status, 0 is executed successfully, 1 is executing, and 2 has failed to execute
	`result` TEXT NOT NULL DEFAULT, -- Result of task execution
	`time` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, -- Task execution time
	PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;


CREATE TABLE `status` (
	`id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
	`device_id` INT NOT NULL,
	`service_name` INT NOT NULL, -- Service name
	`status` TINYINT NOT NULL, -- Service running status
	PRIMARY KEY (`id`),
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
