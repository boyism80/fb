CREATE TABLE IF NOT EXISTS `clan_money_pending` (
  `user` int unsigned NOT NULL,
  `request_id` bigint unsigned NOT NULL,
  `clan` int unsigned NOT NULL,
  `clan_delta` bigint NOT NULL,
  `money_delta` bigint NOT NULL COMMENT 'Negative: held from the character; positive: paid to the character when applied',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`user`,`request_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;
