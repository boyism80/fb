-- @transaction off

INSERT IGNORE INTO `system_storage_box_archive` (
  `id`, `user`, `title`, `message`, `attachments`, `reserve_date`, `expired_date`,
  `include_new_characters`, `canceled`, `external_ref`, `created_date`, `updated_date`)
SELECT `id`, `user`, `title`, `message`, `attachments`, `reserve_date`, `expired_date`,
  `include_new_characters`, `canceled`, `external_ref`, `created_date`, `updated_date`
FROM `system_storage_box` WHERE `deleted` = 1;

DELETE FROM `system_storage_box` WHERE `deleted` = 1;

ALTER TABLE `system_storage_box` DROP COLUMN `deleted`;
