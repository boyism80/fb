-- system_storage_box is broadcast template only; personal grants use shard storage_box.

ALTER TABLE `system_storage_box`
  DROP INDEX `idx_system_storage_user`,
  DROP COLUMN `user`;
