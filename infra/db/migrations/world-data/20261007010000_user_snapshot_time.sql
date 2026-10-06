ALTER TABLE `user`
  ADD COLUMN `snapshot_time` bigint DEFAULT NULL COMMENT 'Unix ms of the game snapshot last saved; older saves are dropped' AFTER `first_login_date`;
