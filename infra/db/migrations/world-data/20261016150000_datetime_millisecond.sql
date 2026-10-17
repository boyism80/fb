-- @transaction off
-- Widen datetime columns to datetime(6). Clock defaults are UTC_TIMESTAMP(6).

ALTER TABLE `user`
  MODIFY `created_date` datetime(6) NOT NULL,
  MODIFY `updated_date` datetime(6) NOT NULL,
  MODIFY `first_login_date` datetime(6) DEFAULT NULL COMMENT 'Set on first successful login; NULL = never logged in';

ALTER TABLE `achievement`
  MODIFY `created_date` datetime(6) NOT NULL,
  MODIFY `updated_date` datetime(6) NOT NULL;

ALTER TABLE `ban`
  MODIFY `expire_date` datetime(6) DEFAULT NULL,
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));

ALTER TABLE `bulletin`
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));

ALTER TABLE `character_realtime_state`
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));

ALTER TABLE `clan`
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));

ALTER TABLE `clan_member`
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));

ALTER TABLE `group`
  MODIFY `created_date` datetime(6) NOT NULL,
  MODIFY `updated_date` datetime(6) NOT NULL;

ALTER TABLE `item`
  MODIFY `expire_time` datetime(6) DEFAULT NULL,
  MODIFY `created_date` datetime(6) NOT NULL,
  MODIFY `updated_date` datetime(6) NOT NULL;

ALTER TABLE `mail`
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));

ALTER TABLE `option`
  MODIFY `created_date` datetime(6) NOT NULL,
  MODIFY `updated_date` datetime(6) NOT NULL;

ALTER TABLE `quest`
  MODIFY `created_date` datetime(6) NOT NULL,
  MODIFY `updated_date` datetime(6) NOT NULL;

ALTER TABLE `spell`
  MODIFY `next` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `created_date` datetime(6) NOT NULL,
  MODIFY `updated_date` datetime(6) NOT NULL;

ALTER TABLE `storage_box`
  MODIFY `expired_date` datetime(6) DEFAULT NULL,
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));

ALTER TABLE `marriage`
  MODIFY `remarriage_after` datetime(6) DEFAULT NULL COMMENT 'Remarriage allowed after this time (set on divorce)',
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));

ALTER TABLE `friend`
  MODIFY `created_date` datetime(6) NOT NULL,
  MODIFY `updated_date` datetime(6) NOT NULL;

ALTER TABLE `matchmaking_skill`
  MODIFY `created_date` datetime(6) NOT NULL,
  MODIFY `updated_date` datetime(6) NOT NULL;

ALTER TABLE `collection_unlock`
  MODIFY `created_date` datetime(6) NOT NULL,
  MODIFY `updated_date` datetime(6) NOT NULL;

ALTER TABLE `clan_alliance`
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));

ALTER TABLE `clan_enemy`
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));

ALTER TABLE `clan_money_request_archive`
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));

ALTER TABLE `unsettled_request`
  MODIFY `created_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6)),
  MODIFY `updated_date` datetime(6) NOT NULL DEFAULT (UTC_TIMESTAMP(6));
