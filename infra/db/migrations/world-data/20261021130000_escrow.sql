-- Money held by an escrow moves from the held item rows to its own row, so an escrow can hold money without an item.
CREATE TABLE IF NOT EXISTS `escrow` (
  `user` int unsigned NOT NULL,
  `id` bigint unsigned NOT NULL,
  `money` bigint unsigned NOT NULL DEFAULT '0',
  `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  PRIMARY KEY (`user`,`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;

INSERT IGNORE INTO `escrow` (`user`, `id`, `money`)
SELECT `owner`, `listing_id`, SUM(`locked_money`)
FROM `item`
WHERE `stored` = -2 AND `listing_id` IS NOT NULL AND `deleted` = 0
GROUP BY `owner`, `listing_id`;

ALTER TABLE `item`
  CHANGE COLUMN `listing_id` `escrow_id` BIGINT UNSIGNED NULL DEFAULT NULL,
  DROP COLUMN `locked_money`;
