ALTER TABLE `clan`
  ADD COLUMN `money_requests` mediumtext DEFAULT NULL COMMENT 'Recent /clan/money request ids and whether each was applied' AFTER `money`;
