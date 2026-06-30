-- Relocate system-wide tables (per-world singletons) from world-data shards to world-global.
-- These are accessed via GetGlobalConnection in the application layer.

CREATE TABLE IF NOT EXISTS `system_mail` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `sender` int unsigned NOT NULL,
  `title` varchar(64) NOT NULL,
  `contents` varchar(256) NOT NULL,
  `expire_date` datetime DEFAULT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;

CREATE TABLE IF NOT EXISTS `system_storage_box` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `user` int unsigned DEFAULT NULL,
  `title` varchar(128) NOT NULL DEFAULT '',
  `message` varchar(256) NOT NULL,
  `attachments` json NOT NULL,
  `expired_date` datetime DEFAULT NULL,
  `external_ref` varchar(128) DEFAULT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `UX_EXTERNAL_REF` (`external_ref`),
  KEY `idx_system_storage_user` (`user`,`deleted`,`expired_date`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
