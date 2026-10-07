-- Not unique: a trade saves both characters separately, so one uid can briefly sit under two owners.
ALTER TABLE `item`
  ADD COLUMN `uid` BIGINT UNSIGNED NULL DEFAULT NULL AFTER `locked_money`,
  ADD KEY `idx_item_uid` (`uid`);
