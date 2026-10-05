-- Storage boxes owed by marketplace purchase, cancel and expire.
-- Rows are inserted in the same transaction as the listing change and delivered after commit;
-- rows with delivered_date IS NULL are retried by MarketplaceDeliveryBackgroundService.
CREATE TABLE IF NOT EXISTS `marketplace_delivery` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  `external_ref` VARCHAR(128) NOT NULL,
  `world` INT UNSIGNED NOT NULL,
  `user` INT UNSIGNED NOT NULL,
  `title` VARCHAR(128) NOT NULL,
  `message` VARCHAR(256) NOT NULL,
  `attachments` JSON NOT NULL,
  `attempts` INT UNSIGNED NOT NULL DEFAULT 0,
  `last_error` VARCHAR(512) NULL DEFAULT NULL,
  `created_date` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `delivered_date` DATETIME NULL DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `UX_MARKETPLACE_DELIVERY_REF` (`external_ref`),
  KEY `IX_MARKETPLACE_DELIVERY_PENDING` (`delivered_date`, `id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
