-- Persist 인품 (signed) and 평가권.
ALTER TABLE `user`
  ADD COLUMN `reputation` SMALLINT NOT NULL DEFAULT 0 AFTER `speed`,
  ADD COLUMN `evaluation` SMALLINT UNSIGNED NOT NULL DEFAULT 0 AFTER `reputation`;
