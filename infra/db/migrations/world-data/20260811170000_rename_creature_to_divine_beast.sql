-- Rename user DIVINE_BEAST storage column from legacy `creature` naming.
-- Castle table is created with `divine_beast` in 20260811160000_castle.sql.

ALTER TABLE `user`
  CHANGE COLUMN `creature` `divine_beast` smallint unsigned DEFAULT NULL;
