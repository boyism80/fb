-- @transaction off
SET FOREIGN_KEY_CHECKS=0;
SET NAMES utf8mb4;

CREATE TABLE `user` (
  `id` int unsigned NOT NULL,
  `world` int unsigned NOT NULL COMMENT 'World id at character creation time; kept for historical/merge lookup',
  `name` varchar(256) NOT NULL,
  `pw` varchar(256) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `birth` int unsigned DEFAULT NULL,
  `role` smallint unsigned NOT NULL DEFAULT '0',
  `look` smallint unsigned NOT NULL DEFAULT '0',
  `color` tinyint unsigned NOT NULL DEFAULT '0',
  `gender` tinyint unsigned NOT NULL DEFAULT '0',
  `nation` smallint unsigned NOT NULL DEFAULT '0',
  `creature` smallint unsigned DEFAULT NULL,
  `map` smallint unsigned NOT NULL DEFAULT '0',
  `position_x` smallint unsigned NOT NULL DEFAULT '0',
  `position_y` smallint unsigned NOT NULL DEFAULT '0',
  `direction` tinyint unsigned NOT NULL DEFAULT '0',
  `state` tinyint unsigned NOT NULL DEFAULT '0',
  `class` tinyint unsigned NOT NULL DEFAULT '0',
  `promotion` tinyint unsigned NOT NULL DEFAULT '0',
  `level` int unsigned NOT NULL DEFAULT '1',
  `exp` int unsigned NOT NULL DEFAULT '0',
  `money` int unsigned NOT NULL DEFAULT '0',
  `deposited_money` int unsigned NOT NULL DEFAULT '0',
  `mimicry` json DEFAULT NULL COMMENT 'Character appearance override (mimicry); NULL = no mimicry',
  `hp` int unsigned NOT NULL,
  `base_hp` int unsigned NOT NULL,
  `additional_hp` int unsigned NOT NULL DEFAULT '0',
  `mp` int unsigned NOT NULL,
  `base_mp` int unsigned NOT NULL,
  `additional_mp` int unsigned NOT NULL DEFAULT '0',
  `weapon_color` tinyint unsigned DEFAULT NULL COMMENT 'Optional override for weapon color (appearance); NULL = use item default',
  `helmet_color` tinyint unsigned DEFAULT NULL,
  `armor_color` tinyint unsigned DEFAULT NULL COMMENT 'Optional override for armor color (appearance); NULL = use item default',
  `shield_color` tinyint unsigned DEFAULT NULL COMMENT 'Optional override for shield color (appearance); NULL = use item default',
  `ring_left_color` tinyint unsigned DEFAULT NULL,
  `ring_right_color` tinyint unsigned DEFAULT NULL,
  `aux_top_color` int unsigned DEFAULT NULL,
  `aux_bot_color` int unsigned DEFAULT NULL,
  `buffs` varchar(512) NOT NULL DEFAULT '[]',
  `title` varchar(32) NOT NULL DEFAULT '',
  `super_hide` tinyint unsigned NOT NULL DEFAULT '0',
  `deleted` tinyint unsigned NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  `first_login_date` datetime DEFAULT NULL COMMENT 'Set on first successful login; NULL = never logged in',
  PRIMARY KEY (`id`),
  UNIQUE KEY `id_UNIQUE` (`id`),
  UNIQUE KEY `name_UNIQUE` (`name`),
  KEY `INDEX_NAME` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;

CREATE TABLE `achievement` (
  `uid` int unsigned NOT NULL,
  `id` int unsigned NOT NULL,
  `text` varchar(128) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '',
  `icon` smallint NOT NULL DEFAULT 0,
  `color` smallint NOT NULL DEFAULT 0,
  `deleted` tinyint DEFAULT NULL,
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  PRIMARY KEY (`uid`,`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;

CREATE TABLE `ban` (
  `user` int unsigned NOT NULL,
  `reason` varchar(256) CHARACTER SET utf8mb3 COLLATE utf8mb3_bin NOT NULL,
  `expire_date` datetime DEFAULT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`user`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;

CREATE TABLE `bulletin` (
  `id` int unsigned NOT NULL,
  `section` int unsigned NOT NULL,
  `user` int unsigned NOT NULL,
  `title` varchar(64) CHARACTER SET utf8mb3 COLLATE utf8mb3_bin NOT NULL,
  `contents` varchar(256) CHARACTER SET utf8mb3 COLLATE utf8mb3_bin NOT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`,`section`),
  KEY `fk.bulletin.owner_idx` (`user`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;

CREATE TABLE `bulletin_sequence` (
  `section` int unsigned NOT NULL,
  `id` int unsigned NOT NULL,
  PRIMARY KEY (`section`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;

CREATE TABLE `character_realtime_state` (
  `uid` int NOT NULL,
  `group` int DEFAULT NULL,
  `clan` int DEFAULT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;

CREATE TABLE `clan` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(45) NOT NULL,
  `title` varchar(45) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci DEFAULT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;

CREATE TABLE `clan_member` (
  `clan` int unsigned NOT NULL,
  `user` int NOT NULL,
  `role` int NOT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`clan`,`user`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;

CREATE TABLE `group` (
  `master` int unsigned NOT NULL,
  `members` varchar(256) NOT NULL,
  `deleted` tinyint NOT NULL,
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  PRIMARY KEY (`master`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;

CREATE TABLE `item` (
  `owner` int unsigned NOT NULL,
  `index` smallint NOT NULL DEFAULT '-1',
  `parts` smallint NOT NULL DEFAULT '0',
  `stored` smallint NOT NULL DEFAULT '-1',
  `model` int unsigned NOT NULL,
  `count` smallint unsigned DEFAULT '1',
  `durability` int unsigned DEFAULT NULL,
  `custom_name` varchar(32) DEFAULT NULL,
  `expire_time` datetime DEFAULT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  PRIMARY KEY (`owner`,`index`,`parts`,`stored`),
  KEY `item_owner_idx` (`owner`),
  CONSTRAINT `fk.item.owner` FOREIGN KEY (`owner`) REFERENCES `user` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;

CREATE TABLE `mail` (
  `id` int unsigned NOT NULL,
  `user` int unsigned NOT NULL,
  `sender` int unsigned NOT NULL,
  `title` varchar(64) NOT NULL,
  `contents` varchar(256) NOT NULL,
  `system_mail_id` int unsigned DEFAULT NULL,
  `read` tinyint NOT NULL DEFAULT '0',
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`,`user`),
  UNIQUE KEY `UX_USER_SYSTEM_MAIL` (`user`,`system_mail_id`),
  KEY `IX_UNAME` (`user`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;

CREATE TABLE `mail_sequence` (
  `user` int NOT NULL,
  `id` int NOT NULL,
  PRIMARY KEY (`user`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;

CREATE TABLE `marketplace_pending` (
  `user` int unsigned NOT NULL,
  `pending_key` varchar(64) CHARACTER SET utf8mb3 COLLATE utf8mb3_bin NOT NULL,
  `type` tinyint unsigned NOT NULL,
  `purchase_id` varchar(64) CHARACTER SET utf8mb3 COLLATE utf8mb3_bin NOT NULL DEFAULT '',
  `listing_id` varchar(64) CHARACTER SET utf8mb3 COLLATE utf8mb3_bin NOT NULL DEFAULT '',
  `attachments` json NOT NULL,
  `expected_purchase_count` smallint unsigned NOT NULL DEFAULT '0',
  `expected_total_price` int unsigned NOT NULL DEFAULT '0',
  `character_id` int unsigned NOT NULL,
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`user`,`pending_key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;

CREATE TABLE `option` (
  `uid` int unsigned NOT NULL,
  `whisper` tinyint unsigned NOT NULL,
  `group` tinyint unsigned NOT NULL,
  `roar` tinyint unsigned NOT NULL,
  `roar_worlds` tinyint unsigned NOT NULL,
  `magic_effect` tinyint unsigned NOT NULL,
  `weather_effect` tinyint unsigned NOT NULL,
  `fixed_move` tinyint unsigned NOT NULL,
  `trade` tinyint unsigned NOT NULL,
  `fast_move` tinyint unsigned NOT NULL,
  `effect_sound` tinyint unsigned NOT NULL,
  `pk_protect` tinyint unsigned NOT NULL,
  `deleted` tinyint unsigned NOT NULL,
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  PRIMARY KEY (`uid`),
  UNIQUE KEY `uid_UNIQUE` (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;

CREATE TABLE `quest` (
  `user` int NOT NULL,
  `id` int NOT NULL,
  `step` int NOT NULL,
  `progress` int NOT NULL,
  `param` varchar(64) DEFAULT NULL,
  `completed` tinyint NOT NULL,
  `deleted` tinyint NOT NULL,
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  PRIMARY KEY (`user`,`id`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;

CREATE TABLE `spell` (
  `owner` int unsigned NOT NULL,
  `slot` tinyint NOT NULL,
  `model` int NOT NULL,
  `next` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  PRIMARY KEY (`owner`,`slot`),
  KEY `spell_owner_idx` (`owner`),
  CONSTRAINT `fk.spell.owner` FOREIGN KEY (`owner`) REFERENCES `user` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;

CREATE TABLE `system_mail` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `sender` int unsigned NOT NULL,
  `title` varchar(64) NOT NULL,
  `contents` varchar(256) NOT NULL,
  `expire_date` datetime DEFAULT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=34 DEFAULT CHARSET=euckr;

CREATE TABLE `storage_box` (
  `user` int unsigned NOT NULL,
  `id` int unsigned NOT NULL,
  `system_storage_box_id` int unsigned DEFAULT NULL,
  `title` varchar(128) NOT NULL DEFAULT '',
  `message` varchar(256) NOT NULL,
  `attachments` json NOT NULL,
  `received` tinyint NOT NULL DEFAULT '0',
  `expired_date` datetime DEFAULT NULL,
  `deleted` tinyint NOT NULL DEFAULT '0',
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`user`,`id`),
  UNIQUE KEY `UX_USER_SYSTEM_STORAGE` (`user`,`system_storage_box_id`),
  KEY `idx_storage_box_expired` (`user`,`expired_date`),
  KEY `idx_storage_box_received` (`user`,`received`,`deleted`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;

CREATE TABLE `system_storage_box` (
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

CREATE TABLE `marriage` (
  `character_id` int unsigned NOT NULL COMMENT 'Character id (one row per character)',
  `spouse_id` int unsigned DEFAULT NULL COMMENT 'Character id of spouse; NULL when single',
  `remarriage_after` datetime DEFAULT NULL COMMENT 'Remarriage allowed after this time (set on divorce)',
  `divorce_count` int unsigned NOT NULL DEFAULT '0' COMMENT 'Number of divorces (for title/achievement)',
  `created_date` datetime NOT NULL DEFAULT (CURRENT_TIMESTAMP),
  `updated_date` datetime NOT NULL DEFAULT (CURRENT_TIMESTAMP) ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`character_id`),
  KEY `idx_marriage_spouse` (`spouse_id`),
  CONSTRAINT `fk.marriage.character` FOREIGN KEY (`character_id`) REFERENCES `user` (`id`) ON DELETE CASCADE,
  CONSTRAINT `fk.marriage.spouse` FOREIGN KEY (`spouse_id`) REFERENCES `user` (`id`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COMMENT='Marriage state per character: spouse_id, remarriage_after, divorce_count';

CREATE TABLE `write_back_failure` (
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

DROP PROCEDURE IF EXISTS `USP_BULLETIN_ADD`;
DELIMITER $$
CREATE PROCEDURE `USP_BULLETIN_ADD`(section INT, uid INT, title NVARCHAR(64), contents NVARCHAR(256))
BEGIN
    DECLARE new_id INT UNSIGNED;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        SELECT 0 AS RESULT;
    END;

    START TRANSACTION;

    SELECT id INTO new_id 
    FROM bulletin_sequence 
    WHERE bulletin_sequence.`section` = section FOR UPDATE;

    IF new_id IS NULL THEN
        SET new_id = 1;
        INSERT INTO bulletin_sequence (`section`, `id`) VALUES (section, new_id);
    ELSE
        SET new_id = new_id + 1;
        UPDATE bulletin_sequence SET id = new_id WHERE bulletin_sequence.`section` = section;
    END IF;

    INSERT INTO bulletin (`id`, `section`, `user`, `title`, `contents`)
    VALUES (new_id, section, uid, title, contents);

    COMMIT;

    SELECT 1 AS RESULT;
    SELECT * FROM bulletin WHERE bulletin.`id` = new_id AND bulletin.`section` = section;
END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS `USP_BULLETIN_DELETE`;
DELIMITER $$
CREATE PROCEDURE `USP_BULLETIN_DELETE`(IN id INT, IN user INT, IN ignore_owner TINYINT)
BEGIN
    DECLARE _deleted TINYINT;
    DECLARE _owner INT;

    SELECT bulletin.deleted, bulletin.user
    INTO _deleted, _owner
    FROM bulletin
    WHERE bulletin.id = id
    LIMIT 1;

    IF _deleted IS NULL THEN
        SELECT -1 AS result;
    ELSEIF _deleted = 1 THEN
        SELECT -2 AS result;
    ELSEIF _owner != user AND ignore_owner = 0 THEN
        SELECT -3 AS result;
    ELSE
        IF ignore_owner = 1 THEN
            UPDATE bulletin SET deleted = 1, updated_date = NOW() WHERE bulletin.id = id;
        ELSE
            UPDATE bulletin SET deleted = 1, updated_date = NOW() WHERE bulletin.id = id AND bulletin.user = user;
        END IF;
        SELECT 1 AS result;
    END IF;
END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS `USP_BULLETIN_GET`;
DELIMITER $$
CREATE PROCEDURE `USP_BULLETIN_GET`(IN section INT, IN article INT)
BEGIN
    SELECT A.`id`,
           A.`user`,
           A.`title`,
           A.`contents`,
           A.`created_date` 
    FROM `bulletin` AS A
    WHERE A.`id` = article AND A.`section` = section AND `deleted` = 0;
    
    SELECT EXISTS(SELECT * FROM `bulletin`
                  WHERE `bulletin`.`section` = section 
                    AND `bulletin`.`id` > article 
                    AND `bulletin`.`deleted` = 0 
                  LIMIT 1) as `next`;
END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS `USP_BULLETIN_GET_LIST`;
DELIMITER $$
CREATE PROCEDURE `USP_BULLETIN_GET_LIST`(IN section INT, IN position INT)
BEGIN
    SELECT A.`id`,
           A.`section`,
           A.`user`,
           A.`title`,
           A.`created_date`
    FROM bulletin AS A
    WHERE A.`section` = section AND A.`deleted` = 0 AND position >= A.`id`
    ORDER BY A.`id` DESC
    LIMIT 0, 20;
END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS `USP_BULLETIN_UPDATE`;
DELIMITER $$
CREATE PROCEDURE `USP_BULLETIN_UPDATE`(
    IN p_id INT,
    IN p_user INT,
    IN p_title NVARCHAR(64),
    IN p_contents NVARCHAR(256),
    IN p_ignore_owner TINYINT
)
BEGIN
    DECLARE _deleted TINYINT;
    DECLARE _owner INT;

    SELECT bulletin.deleted, bulletin.user
    INTO _deleted, _owner
    FROM bulletin
    WHERE bulletin.id = p_id
    LIMIT 1;

    IF _deleted IS NULL THEN
        SELECT -1 AS result;
    ELSEIF _deleted = 1 THEN
        SELECT -2 AS result;
    ELSEIF _owner != p_user AND p_ignore_owner = 0 THEN
        SELECT -3 AS result;
    ELSE
        IF p_ignore_owner = 1 THEN
            UPDATE bulletin
            SET title = p_title,
                contents = p_contents,
                updated_date = NOW()
            WHERE bulletin.id = p_id;
        ELSE
            UPDATE bulletin
            SET title = p_title,
                contents = p_contents,
                updated_date = NOW()
            WHERE bulletin.id = p_id AND bulletin.user = p_user;
        END IF;

        IF ROW_COUNT() = 0 THEN
            SELECT -4 AS result;
        ELSE
            SELECT 1 AS result;
        END IF;
    END IF;
END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS `USP_MAIL_COUNT_BY_USER`;
DELIMITER $$
CREATE PROCEDURE `USP_MAIL_COUNT_BY_USER`(
    IN user INT,
    IN mail_filter TINYINT
)
BEGIN
    SELECT COUNT(*)
    FROM mail
    WHERE mail.`user` = user
      AND mail.`deleted` = 0
      AND (
          mail_filter = 0
          OR (mail_filter = 1 AND mail.`read` = 0)
          OR (mail_filter = 2 AND mail.`system_mail_id` IS NOT NULL)
      );
END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS `USP_MAIL_DELIVER_SYSTEM_MANY`;
DELIMITER $$
CREATE PROCEDURE `USP_MAIL_DELIVER_SYSTEM_MANY`(
    IN system_mail_id INT UNSIGNED,
    IN sender INT
)
BEGIN
    DECLARE done INT DEFAULT FALSE;
    DECLARE v_user INT UNSIGNED;
    DECLARE v_title NVARCHAR(64);
    DECLARE v_contents NVARCHAR(256);
    DECLARE new_id INT UNSIGNED;

    -- Caller must populate session temp tables on the same connection:
    --   tmp_mail_write_users (user_id)
    --   tmp_mail_deliver_content (user_id, title, contents) — per-recipient rendered text

    DECLARE user_cursor CURSOR FOR
        SELECT u.user_id, c.title, c.contents
        FROM tmp_mail_write_users u
        INNER JOIN tmp_mail_deliver_content c ON c.user_id = u.user_id
        WHERE u.user_id > 0
        ORDER BY u.user_id;

    DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = TRUE;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        SELECT 0 AS RESULT;
    END;

    DROP TEMPORARY TABLE IF EXISTS tmp_mail_written;
    CREATE TEMPORARY TABLE tmp_mail_written (
        `user` INT UNSIGNED NOT NULL,
        `id` INT UNSIGNED NOT NULL,
        PRIMARY KEY (`user`, `id`)
    ) ENGINE = MEMORY;

    START TRANSACTION;

    OPEN user_cursor;

    read_loop: LOOP
        FETCH user_cursor INTO v_user, v_title, v_contents;
        IF done THEN
            LEAVE read_loop;
        END IF;

        IF EXISTS (
            SELECT 1 FROM mail
            WHERE mail.`user` = v_user
              AND mail.system_mail_id = system_mail_id
              AND mail.deleted = 0
        ) THEN
            ITERATE read_loop;
        END IF;

        SELECT id INTO new_id
        FROM mail_sequence
        WHERE mail_sequence.`user` = v_user FOR UPDATE;

        IF new_id IS NULL THEN
            SET new_id = 1;
            INSERT INTO mail_sequence (`user`, `id`) VALUES (v_user, new_id);
        ELSE
            SET new_id = new_id + 1;
            UPDATE mail_sequence SET id = new_id WHERE mail_sequence.`user` = v_user;
        END IF;

        INSERT INTO mail (`id`, `user`, `sender`, `title`, `contents`, `system_mail_id`)
        VALUES (new_id, v_user, sender, v_title, v_contents, system_mail_id);

        INSERT INTO tmp_mail_written (`user`, `id`) VALUES (v_user, new_id);
    END LOOP;

    CLOSE user_cursor;

    COMMIT;

    SELECT 1 AS RESULT;

    SELECT m.*
    FROM mail m
    INNER JOIN tmp_mail_written t ON m.`user` = t.`user` AND m.`id` = t.`id`;

    SELECT m.`user`, CAST(COUNT(*) AS UNSIGNED) AS unread
    FROM mail m
    INNER JOIN tmp_mail_written t ON m.`user` = t.`user`
    WHERE m.`read` = 0 AND m.deleted = 0
    GROUP BY m.`user`;

    DROP TEMPORARY TABLE tmp_mail_written;
END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS `USP_MAIL_GET_LIST`;
DELIMITER $$
CREATE PROCEDURE `USP_MAIL_GET_LIST`(
    IN user INT,
    IN offset INT,
    IN count INT,
    IN mail_filter TINYINT
)
BEGIN
    SELECT mail.`id`,
           mail.`user`,
           mail.`sender`,
           mail.`title`,
           mail.`contents`,
           mail.`system_mail_id`,
           mail.`read`,
           mail.`deleted`,
           mail.`created_date`,
           mail.`updated_date`
    FROM mail
    WHERE mail.`user` = user
      AND mail.`deleted` = 0
      AND (
          mail_filter = 0
          OR (mail_filter = 1 AND mail.`read` = 0)
          OR (mail_filter = 2 AND mail.`system_mail_id` IS NOT NULL)
      )
    ORDER BY mail.`id` DESC
    LIMIT offset, count;
END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS `USP_MAIL_GET_SUMMARY_LIST`;
DELIMITER $$
CREATE PROCEDURE `USP_MAIL_GET_SUMMARY_LIST`(IN user INT, IN position INT, IN count INT)
BEGIN
    SELECT mail.`id`,
           mail.`user`,
           mail.`sender`,
           mail.`title`,
           mail.`read`,
           mail.`created_date`
    FROM mail
    WHERE mail.`user` = user AND mail.`deleted` = 0 AND position >= mail.`id`
    ORDER BY mail.`id` DESC
    LIMIT 0, count;
END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS `USP_MAIL_READ`;
DELIMITER $$
CREATE PROCEDURE `USP_MAIL_READ`(IN user INT, IN id INT)
BEGIN
	  START TRANSACTION;
      
      UPDATE mail
	  SET `read` = 1
	  WHERE mail.`id` = id AND 
			mail.`user` = user AND 
			mail.`deleted` = 0;

	SELECT * FROM `mail`
    WHERE mail.`id` = id AND
          mail.`user` = user AND
          mail.`deleted` = 0;
          
	COMMIT;
END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS `USP_MAIL_WRITE`;
DELIMITER $$
CREATE PROCEDURE `USP_MAIL_WRITE`(
    IN user INT, 
    IN sender INT, 
    IN title NVARCHAR(64), 
    IN contents NVARCHAR(256)
)
BEGIN
    DECLARE new_id INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        SELECT 0 AS RESULT;
    END;

    START TRANSACTION;

    SELECT id INTO new_id 
    FROM mail_sequence 
    WHERE mail_sequence.`user` = user FOR UPDATE;

    IF new_id IS NULL THEN
        SET new_id = 1;
        INSERT INTO mail_sequence (`user`, `id`) VALUES (user, new_id);
    ELSE
        SET new_id = new_id + 1;
        UPDATE mail_sequence SET id = new_id WHERE mail_sequence.`user` = user;
    END IF;

    INSERT INTO mail (`id`, `user`, `sender`, `title`, `contents`)
    VALUES (new_id, user, sender, title, contents);

    COMMIT;

    SELECT 1 AS RESULT;
    SELECT * FROM mail WHERE mail.`id` = new_id AND mail.`user` = user;
END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS `USP_MAIL_WRITE_MANY`;
DELIMITER $$
CREATE PROCEDURE `USP_MAIL_WRITE_MANY`(
    IN sender INT,
    IN title NVARCHAR(64),
    IN contents NVARCHAR(256)
)
BEGIN
    DECLARE done INT DEFAULT FALSE;
    DECLARE v_user INT UNSIGNED;
    DECLARE new_id INT UNSIGNED;

    -- Caller must populate session temp table tmp_mail_write_users (user_id) on the same connection.

    DECLARE user_cursor CURSOR FOR
        SELECT user_id
        FROM tmp_mail_write_users
        WHERE user_id > 0
        ORDER BY user_id;

    DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = TRUE;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        SELECT 0 AS RESULT;
    END;

    DROP TEMPORARY TABLE IF EXISTS tmp_mail_written;
    CREATE TEMPORARY TABLE tmp_mail_written (
        `user` INT UNSIGNED NOT NULL,
        `id` INT UNSIGNED NOT NULL,
        PRIMARY KEY (`user`, `id`)
    ) ENGINE = MEMORY;

    START TRANSACTION;

    OPEN user_cursor;

    read_loop: LOOP
        FETCH user_cursor INTO v_user;
        IF done THEN
            LEAVE read_loop;
        END IF;

        SELECT id INTO new_id
        FROM mail_sequence
        WHERE mail_sequence.`user` = v_user FOR UPDATE;

        IF new_id IS NULL THEN
            SET new_id = 1;
            INSERT INTO mail_sequence (`user`, `id`) VALUES (v_user, new_id);
        ELSE
            SET new_id = new_id + 1;
            UPDATE mail_sequence SET id = new_id WHERE mail_sequence.`user` = v_user;
        END IF;

        INSERT INTO mail (`id`, `user`, `sender`, `title`, `contents`)
        VALUES (new_id, v_user, sender, title, contents);

        INSERT INTO tmp_mail_written (`user`, `id`) VALUES (v_user, new_id);
    END LOOP;

    CLOSE user_cursor;

    COMMIT;

    SELECT 1 AS RESULT;

    SELECT m.*
    FROM mail m
    INNER JOIN tmp_mail_written t ON m.`user` = t.`user` AND m.`id` = t.`id`;

    SELECT m.`user`, CAST(COUNT(*) AS UNSIGNED) AS unread
    FROM mail m
    INNER JOIN tmp_mail_written t ON m.`user` = t.`user`
    WHERE m.`read` = 0 AND m.deleted = 0
    GROUP BY m.`user`;

    DROP TEMPORARY TABLE tmp_mail_written;
END$$
DELIMITER ;

SET FOREIGN_KEY_CHECKS=1;
