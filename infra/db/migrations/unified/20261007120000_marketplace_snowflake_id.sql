-- Listing and purchase ids are snowflake ids issued by the game server; existing UUID rows cannot be converted.
DELETE FROM `marketplace_purchase`;
DELETE FROM `marketplace_listing`;
DELETE FROM `marketplace_listing_archive`;

ALTER TABLE `marketplace_listing`
  MODIFY COLUMN `id` BIGINT UNSIGNED NOT NULL;

ALTER TABLE `marketplace_listing_archive`
  MODIFY COLUMN `id` BIGINT UNSIGNED NOT NULL;

ALTER TABLE `marketplace_purchase`
  DROP INDEX `uk_id`,
  MODIFY COLUMN `id` BIGINT UNSIGNED NOT NULL COMMENT 'Purchase ID (snowflake issued by game server)',
  MODIFY COLUMN `listing_id` BIGINT UNSIGNED NOT NULL;
