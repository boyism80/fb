ALTER TABLE `marketplace_listing`
  ADD COLUMN `item_uid` BIGINT UNSIGNED NULL DEFAULT NULL AFTER `item_custom_name`,
  ADD KEY `idx_item_uid` (`item_uid`);

ALTER TABLE `marketplace_listing_archive`
  ADD COLUMN `item_uid` BIGINT UNSIGNED NULL DEFAULT NULL AFTER `item_custom_name`,
  ADD KEY `idx_item_uid` (`item_uid`);
