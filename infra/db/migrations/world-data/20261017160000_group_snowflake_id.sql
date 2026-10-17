-- @transaction off
-- Groups are keyed by a game server snowflake id instead of the master uid. Group rows only live while a party exists.

UPDATE `character_realtime_state` SET `group` = NULL WHERE `group` IS NOT NULL;

ALTER TABLE `character_realtime_state`
  MODIFY `group` bigint unsigned DEFAULT NULL;

DROP TABLE IF EXISTS `group`;

CREATE TABLE `group` (
  `id` bigint unsigned NOT NULL COMMENT 'Game server snowflake id; its world bits name the world the group belongs to',
  `master` int unsigned NOT NULL,
  `members` varchar(256) NOT NULL,
  `revision` bigint unsigned NOT NULL DEFAULT 0 COMMENT 'Bumped on every group change; game servers apply update messages in this order',
  `deleted` tinyint NOT NULL,
  `created_date` datetime(6) NOT NULL,
  `updated_date` datetime(6) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `group_master_idx` (`master`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
