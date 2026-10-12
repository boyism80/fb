CREATE TABLE IF NOT EXISTS `storage_claim_pending` (
  `user` int unsigned NOT NULL,
  `box` int unsigned NOT NULL,
  `claim_id` bigint unsigned NOT NULL,
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`user`,`box`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;
