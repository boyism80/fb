ALTER TABLE `clan`
  ADD COLUMN `money` bigint unsigned NOT NULL DEFAULT 0 AFTER `title`;
