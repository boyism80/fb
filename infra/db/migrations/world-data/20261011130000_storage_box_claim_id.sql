ALTER TABLE `storage_box`
  ADD COLUMN `claim_id` bigint unsigned DEFAULT NULL COMMENT 'Game claim attempt that set received' AFTER `received`;
