-- @transaction off
-- Widen datetime columns to datetime(6). Clock defaults are UTC_TIMESTAMP(6).

ALTER TABLE `marketplace_listing`
  MODIFY `expire_date` datetime(6) NOT NULL,
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `sold_date` datetime(6) DEFAULT NULL COMMENT 'Set when remaining_count becomes 0';

ALTER TABLE `marketplace_listing_archive`
  MODIFY `expire_date` datetime(6) NOT NULL,
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `sold_date` datetime(6) DEFAULT NULL COMMENT 'Set when remaining_count becomes 0',
  MODIFY `archived_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)) COMMENT 'Date when listing was archived';

ALTER TABLE `marketplace_purchase`
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));

ALTER TABLE `marketplace_statistics`
  MODIFY `last_updated` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));

ALTER TABLE `marketplace_parcel`
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `sent_date` datetime(6) NULL DEFAULT NULL;

ALTER TABLE `name_registry`
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `reserved_at` datetime(6) NULL DEFAULT NULL;

ALTER TABLE `clan_name`
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));
