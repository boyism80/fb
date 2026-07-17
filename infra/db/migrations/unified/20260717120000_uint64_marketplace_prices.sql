-- Widen marketplace listing and purchase prices to 64-bit.
ALTER TABLE `marketplace_listing`
  MODIFY COLUMN `price` BIGINT UNSIGNED NOT NULL COMMENT 'Per unit price set by seller';

ALTER TABLE `marketplace_listing_archive`
  MODIFY COLUMN `price` BIGINT UNSIGNED NOT NULL COMMENT 'Per unit price set by seller';

ALTER TABLE `marketplace_purchase`
  MODIFY COLUMN `purchase_price` BIGINT UNSIGNED NOT NULL COMMENT 'Total price paid (purchase_count * unit_price)';

ALTER TABLE `marketplace_statistics`
  MODIFY COLUMN `avg_price` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  MODIFY COLUMN `min_price` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  MODIFY COLUMN `max_price` BIGINT UNSIGNED NOT NULL DEFAULT 0;
