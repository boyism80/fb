-- Escrow rows hold items and listing fee locked by a pending marketplace list request.
ALTER TABLE `item`
  ADD COLUMN `listing_id` VARCHAR(36) NULL DEFAULT NULL AFTER `expire_time`,
  ADD COLUMN `locked_money` BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER `listing_id`;
