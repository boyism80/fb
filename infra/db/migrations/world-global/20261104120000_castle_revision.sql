-- @transaction off

ALTER TABLE `castle`
  ADD COLUMN `revision` bigint unsigned NOT NULL DEFAULT 0 COMMENT 'Raised by every write of the row; game hosts ignore an update older than the one they hold' AFTER `siege_ended`;
