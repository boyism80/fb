ALTER TABLE `clan`
  ADD COLUMN `revision` bigint unsigned NOT NULL DEFAULT 0 COMMENT 'Bumped on every clan change; game servers apply update messages in this order' AFTER `money_requests`;

ALTER TABLE `group`
  ADD COLUMN `revision` bigint unsigned NOT NULL DEFAULT 0 COMMENT 'Bumped on every group change; game servers apply update messages in this order' AFTER `members`;
