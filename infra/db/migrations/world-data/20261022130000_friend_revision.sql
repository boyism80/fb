CREATE TABLE IF NOT EXISTS `friend_revision` (
  `user` int unsigned NOT NULL,
  `revision` bigint unsigned NOT NULL DEFAULT '0',
  `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  PRIMARY KEY (`user`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;
