-- Aborted rows are tombstones that block a late purchase request; abort-purchase settles a purchase by this key.
-- Purchase rows outlive their listing: archiving a listing must not delete the record abort-purchase relies on.
ALTER TABLE `marketplace_purchase`
  DROP FOREIGN KEY `fk_purchase_listing`,
  ADD COLUMN `aborted` TINYINT(1) NOT NULL DEFAULT 0 COMMENT '1=Tombstone written by abort-purchase' AFTER `purchase_price`;
