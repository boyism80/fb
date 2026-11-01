-- @transaction off

ALTER TABLE `castle`
  ADD COLUMN `siege_id` bigint unsigned DEFAULT NULL COMMENT 'Siege round number, +1 per started round; hosts apply a siege update only when it is not older than their round' AFTER `owner_clan`,
  ADD COLUMN `siege_key` bigint unsigned DEFAULT NULL COMMENT 'Start key sent by the game server (scheduled slot or GM start time); a start with the same key is ignored' AFTER `siege_id`,
  ADD COLUMN `siege_end_at` datetime(6) DEFAULT NULL COMMENT 'UTC time the siege round ends; GM finish moves it to now' AFTER `siege_key`,
  ADD COLUMN `siege_ended` tinyint NOT NULL DEFAULT 0 COMMENT '1 once the heart host saved the result of siege_id' AFTER `siege_end_at`;
