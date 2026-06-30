-- @transaction off
SET FOREIGN_KEY_CHECKS=0;
SET NAMES utf8mb4;

CREATE TABLE `clan_name` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(256) NOT NULL,
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name_UNIQUE` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;

CREATE TABLE `name_registry` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `world` int unsigned NOT NULL COMMENT 'World id at reservation time; kept for historical/merge lookup',
  `name` varchar(256) CHARACTER SET utf8mb3 COLLATE utf8mb3_bin NOT NULL,
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_bin;

DROP PROCEDURE IF EXISTS `USP_CLAN_NAME_DELETE`;
DELIMITER $$
CREATE PROCEDURE `USP_CLAN_NAME_DELETE`(id INT)
BEGIN
	DELETE FROM `clan_name` WHERE `clan_name`.`id` = id;
END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS `USP_NAME_GET_ID`;
DELIMITER $$
CREATE PROCEDURE `USP_NAME_GET_ID`(n NVARCHAR(256))
BEGIN
	SELECT id FROM name_registry WHERE name_registry.name = n;
END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS `USP_NAME_SET`;
DELIMITER $$
CREATE PROCEDURE `USP_NAME_SET`(IN uname NVARCHAR(256), IN in_world INT UNSIGNED)
BEGIN
    DECLARE uid INT;
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
		ROLLBACK;
        SELECT 0 AS result, 0;
    END;

	SET TRANSACTION ISOLATION LEVEL REPEATABLE READ;
    START TRANSACTION;
    
    SELECT id INTO uid FROM name_registry WHERE name = uname FOR UPDATE;
    
    IF uid IS NULL THEN
        INSERT INTO name_registry (name, world) VALUES (uname, in_world);
        SET uid = LAST_INSERT_ID();
        SELECT 1 AS result, uid;
    ELSE
        SELECT 0 AS result, uid;
    END IF;

    COMMIT;
END$$
DELIMITER ;

SET FOREIGN_KEY_CHECKS=1;
