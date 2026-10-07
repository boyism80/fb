-- Listing and purchase ids are snowflake ids issued by the game server; existing UUID rows cannot be converted.
DELETE FROM `item` WHERE `listing_id` IS NOT NULL;
DELETE FROM `marketplace_pending`;

ALTER TABLE `item`
  MODIFY COLUMN `listing_id` BIGINT UNSIGNED NULL DEFAULT NULL;

ALTER TABLE `marketplace_pending`
  MODIFY COLUMN `pending_key` BIGINT UNSIGNED NOT NULL,
  MODIFY COLUMN `purchase_id` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  MODIFY COLUMN `listing_id` BIGINT UNSIGNED NOT NULL DEFAULT 0;
