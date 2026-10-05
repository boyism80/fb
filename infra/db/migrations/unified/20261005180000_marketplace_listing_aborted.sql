-- ABORTED rows are tombstones that block a late list request; the archive job pins rows by (status, updated_date).
ALTER TABLE `marketplace_listing`
  MODIFY COLUMN `status` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0=Active, 1=Sold, 2=Cancelled, 3=Expired, 4=Aborted',
  ADD KEY `idx_status_updated` (`status`, `updated_date`);

ALTER TABLE `marketplace_listing_archive`
  MODIFY COLUMN `status` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0=Active, 1=Sold, 2=Cancelled, 3=Expired, 4=Aborted';
