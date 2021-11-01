-- MIT License Copyright (c) 2021, h1zzz

CREATE TABLE `device` (
	`id` INT unsigned NOT NULL AUTO_INCREMENT, -- Device unique ID'
	KEY `device_index` (`id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
