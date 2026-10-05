-- A retried marketplace delivery finds the box it already created through external_ref.
ALTER TABLE `storage_box`
  ADD COLUMN `external_ref` VARCHAR(128) NULL DEFAULT NULL AFTER `system_storage_box_id`;
