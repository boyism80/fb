CREATE TABLE IF NOT EXISTS `storage_box_legacy_ref` (
  `user` int unsigned NOT NULL,
  `external_ref` varchar(128) NOT NULL COMMENT 'External ref of a box written before boxes moved to system_storage_box; a resent marketplace parcel with this ref is already sent',
  PRIMARY KEY (`user`, `external_ref`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;

INSERT IGNORE INTO `storage_box_legacy_ref` (`user`, `external_ref`)
SELECT `user`, `external_ref` FROM `storage_box`
WHERE `external_ref` IS NOT NULL;
