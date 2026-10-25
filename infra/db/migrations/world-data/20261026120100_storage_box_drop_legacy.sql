-- @transaction off

ALTER TABLE `storage_box`
  DROP INDEX `idx_storage_box_received`,
  DROP COLUMN `external_ref`,
  DROP COLUMN `claim_id`,
  DROP COLUMN `deleted`;

ALTER TABLE `unsettled_request`
  MODIFY COLUMN `type` tinyint unsigned NOT NULL COMMENT '0: clan money, 2: marketplace list, 3: marketplace purchase, 4: clan fee';

DROP TABLE IF EXISTS `storage_box_sequence`;
DROP TABLE IF EXISTS `storage_box_legacy_ref`;
DROP PROCEDURE IF EXISTS `USP_STORAGE_BOX_WRITE`;
DROP PROCEDURE IF EXISTS `USP_STORAGE_BOX_DELIVER_SYSTEM_MANY`;
