ALTER TABLE `user`
  ADD COLUMN `strength` TINYINT UNSIGNED NOT NULL DEFAULT 3 COMMENT 'Base strength without equipment or buffs' AFTER `additional_mp`,
  ADD COLUMN `dexterity` TINYINT UNSIGNED NOT NULL DEFAULT 3 COMMENT 'Base dexterity without equipment or buffs' AFTER `strength`,
  ADD COLUMN `intelligence` TINYINT UNSIGNED NOT NULL DEFAULT 3 COMMENT 'Base intelligence without equipment or buffs' AFTER `dexterity`;
