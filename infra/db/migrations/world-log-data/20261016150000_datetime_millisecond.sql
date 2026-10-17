-- @transaction off
-- Widen datetime columns to datetime(6). Clock defaults are UTC_TIMESTAMP(6).

ALTER TABLE `log`
  MODIFY `timestamp` datetime(6) NOT NULL,
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));
