-- Persist character walk speed (update_internal trailer walk_speed).
ALTER TABLE `user`
  ADD COLUMN `speed` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `super_hide`;
