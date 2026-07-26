-- Rename option.roar_worlds to news to match OPTION.NEWS / FlatBuffer field.
ALTER TABLE `option`
  CHANGE COLUMN `roar_worlds` `news` tinyint unsigned NOT NULL;
