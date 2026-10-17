-- @transaction off
-- Widen datetime columns to datetime(6). Clock defaults are UTC_TIMESTAMP(6).

ALTER TABLE `system_mail`
  MODIFY `expire_date` datetime(6) DEFAULT NULL,
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));

ALTER TABLE `system_storage_box`
  MODIFY `expired_date` datetime(6) DEFAULT NULL,
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));

ALTER TABLE `write_back_failure`
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));

ALTER TABLE `castle`
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));
