ALTER TABLE `option`
  ADD COLUMN `super_hide` tinyint unsigned NOT NULL DEFAULT 0 AFTER `visible_helmet`;

UPDATE `option` AS o
JOIN `user` AS u ON u.`id` = o.`uid`
SET o.`super_hide` = u.`super_hide`;

ALTER TABLE `user`
  DROP COLUMN `super_hide`;
