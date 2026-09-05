-- Rename user.look to hair (HAIR.TBL / character hair).
ALTER TABLE `user`
  CHANGE COLUMN `look` `hair` SMALLINT UNSIGNED NOT NULL;
