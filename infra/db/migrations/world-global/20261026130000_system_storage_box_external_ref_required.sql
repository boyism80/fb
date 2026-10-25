-- @transaction off

UPDATE `system_storage_box` SET `external_ref` = CONCAT('system:', `id`) WHERE `external_ref` IS NULL;
UPDATE `system_storage_box_archive` SET `external_ref` = CONCAT('system:', `id`) WHERE `external_ref` IS NULL;

ALTER TABLE `system_storage_box`
  MODIFY COLUMN `external_ref` varchar(128) NOT NULL COMMENT 'Idempotency key of the grant request; a resend with the same key returns the existing box';

ALTER TABLE `system_storage_box_archive`
  MODIFY COLUMN `external_ref` varchar(128) NOT NULL COMMENT 'Idempotency key of the grant request; a resend with the same key returns the existing box';
