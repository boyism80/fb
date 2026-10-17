-- @transaction off

ALTER TABLE `system_mail_archive`
  ADD COLUMN `purged` tinyint NOT NULL DEFAULT 0 COMMENT 'Deleted mail rows and received records of this mail are removed from every data shard' AFTER `canceled`,
  ADD INDEX `IX_SYSTEM_MAIL_ARCHIVE_PURGED` (`purged`);
