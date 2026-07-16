-- Relocate write_back_failure from world-data shards to world-global.
-- WriteBackService records failures via GetGlobalConnection.

CREATE TABLE IF NOT EXISTS `write_back_failure` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `world` int unsigned NOT NULL,
  `db_shard` int NOT NULL COMMENT 'Write-back queue DB: -1 global, else data shard index',
  `hash` int unsigned DEFAULT NULL,
  `redis_key` varchar(256) NOT NULL,
  `sql_text` mediumtext NOT NULL,
  `error_code` int NOT NULL DEFAULT 0,
  `sql_state` varchar(10) DEFAULT NULL,
  `error_message` varchar(2048) NOT NULL,
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `status` tinyint unsigned NOT NULL DEFAULT 0 COMMENT '0=pending, 1=replayed, 2=discarded',
  PRIMARY KEY (`id`),
  KEY `idx_write_back_failure_world_created` (`world`,`created_date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COMMENT='Write-back failures (no MQ retry; ops replay)';
