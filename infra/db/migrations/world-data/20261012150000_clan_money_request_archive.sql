CREATE TABLE IF NOT EXISTS `clan_money_request_archive` (
  `clan` int unsigned NOT NULL,
  `request_id` bigint unsigned NOT NULL,
  `applied` tinyint(1) NOT NULL,
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`clan`,`request_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;
