-- @transaction off

ALTER TABLE `system_storage_box`
  ADD COLUMN `user` int unsigned DEFAULT NULL COMMENT 'Recipient character; NULL sends to every character' AFTER `id`,
  ADD INDEX `IX_SYSTEM_STORAGE_BOX_USER` (`user`);

ALTER TABLE `system_storage_box_archive`
  ADD COLUMN `user` int unsigned DEFAULT NULL COMMENT 'Recipient character; NULL sent to every character' AFTER `id`;
