-- Persist 6.51 FACE.TBL index (unused by 5.50/5.65 clients).
ALTER TABLE `user`
  ADD COLUMN `face` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `look`;
