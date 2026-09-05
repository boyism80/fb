-- Persist 6.51 appearance ridable_id (mount type). Default 1 is the horse.
ALTER TABLE `user`
  ADD COLUMN `ridable_id` SMALLINT UNSIGNED NOT NULL DEFAULT 1 AFTER `face`;
