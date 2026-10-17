-- @transaction off

ALTER TABLE `system_mail`
  ADD COLUMN `reserve_date` datetime(6) DEFAULT NULL COMMENT 'Sent from this time; NULL sends on creation' AFTER `contents`,
  ADD COLUMN `include_new_characters` tinyint NOT NULL DEFAULT 0 COMMENT 'Also sent to characters created after the send time' AFTER `expire_date`,
  ADD COLUMN `canceled` tinyint NOT NULL DEFAULT 0 AFTER `include_new_characters`;

ALTER TABLE `system_storage_box`
  ADD COLUMN `reserve_date` datetime(6) DEFAULT NULL COMMENT 'Sent from this time; NULL sends on creation' AFTER `attachments`,
  ADD COLUMN `include_new_characters` tinyint NOT NULL DEFAULT 0 COMMENT 'Also sent to characters created after the send time' AFTER `expired_date`,
  ADD COLUMN `canceled` tinyint NOT NULL DEFAULT 0 AFTER `include_new_characters`;

CREATE TABLE IF NOT EXISTS `system_mail_archive` (
  `id` int unsigned NOT NULL,
  `sender` int unsigned NOT NULL,
  `title` varchar(64) NOT NULL,
  `contents` varchar(256) NOT NULL,
  `reserve_date` datetime(6) DEFAULT NULL,
  `expire_date` datetime(6) DEFAULT NULL,
  `include_new_characters` tinyint NOT NULL DEFAULT 0,
  `canceled` tinyint NOT NULL DEFAULT 0,
  `created_date` datetime(6) NOT NULL,
  `updated_date` datetime(6) NOT NULL,
  `archived_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;

CREATE TABLE IF NOT EXISTS `system_storage_box_archive` (
  `id` int unsigned NOT NULL,
  `title` varchar(128) NOT NULL DEFAULT '',
  `message` varchar(256) NOT NULL,
  `attachments` json NOT NULL,
  `reserve_date` datetime(6) DEFAULT NULL,
  `expired_date` datetime(6) DEFAULT NULL,
  `include_new_characters` tinyint NOT NULL DEFAULT 0,
  `canceled` tinyint NOT NULL DEFAULT 0,
  `external_ref` varchar(128) DEFAULT NULL,
  `created_date` datetime(6) NOT NULL,
  `updated_date` datetime(6) NOT NULL,
  `archived_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  PRIMARY KEY (`id`),
  UNIQUE KEY `UX_EXTERNAL_REF` (`external_ref`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
