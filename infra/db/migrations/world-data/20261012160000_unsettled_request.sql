CREATE TABLE IF NOT EXISTS `unsettled_request` (
  `user` int unsigned NOT NULL,
  `id` bigint unsigned NOT NULL,
  `type` tinyint unsigned NOT NULL COMMENT '0: clan money, 1: storage claim, 2: marketplace list, 3: marketplace purchase',
  `target` bigint unsigned NOT NULL DEFAULT '0' COMMENT 'Clan id, storage box id, or listing id',
  `detail` json NOT NULL,
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`user`,`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;

INSERT IGNORE INTO `unsettled_request` (`user`, `id`, `type`, `target`, `detail`)
SELECT `user`, `request_id`, 0, `clan`, JSON_OBJECT('clan_delta', `clan_delta`, 'money_delta', `money_delta`)
FROM `clan_money_pending`;

INSERT IGNORE INTO `unsettled_request` (`user`, `id`, `type`, `target`, `detail`)
SELECT `user`, `claim_id`, 1, `box`, JSON_OBJECT()
FROM `storage_claim_pending`;

INSERT IGNORE INTO `unsettled_request` (`user`, `id`, `type`, `target`, `detail`)
SELECT `owner`, `listing_id`, 2, 0, JSON_OBJECT()
FROM `item`
WHERE `stored` = -2 AND `listing_id` IS NOT NULL AND `deleted` = 0
GROUP BY `owner`, `listing_id`;

INSERT IGNORE INTO `unsettled_request` (`user`, `id`, `type`, `target`, `detail`)
SELECT `user`, `pending_key`, 2, 0, JSON_OBJECT('attachments', `attachments`)
FROM `marketplace_pending`
WHERE `type` = 0;

INSERT IGNORE INTO `unsettled_request` (`user`, `id`, `type`, `target`, `detail`)
SELECT `user`, `pending_key`, 3, `listing_id`, JSON_OBJECT('count', `expected_purchase_count`, 'price', `expected_total_price`)
FROM `marketplace_pending`
WHERE `type` = 1;

DROP TABLE IF EXISTS `clan_money_pending`;
DROP TABLE IF EXISTS `storage_claim_pending`;
DROP TABLE IF EXISTS `marketplace_pending`;
