ALTER TABLE `ban`
  ADD COLUMN `actor_role` TINYINT UNSIGNED NULL DEFAULT NULL COMMENT 'Role of the operator who placed the ban; only that role or higher may replace or lift it' AFTER `expire_date`;
