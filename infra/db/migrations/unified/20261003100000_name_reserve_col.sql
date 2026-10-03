-- Replace Redis-based name reservation with DB-column approach.
-- reserved_at IS NULL     = confirmed permanent registration
-- reserved_at IS NOT NULL = pending reservation (refreshed by keep-alive)
-- Expired when: TIMESTAMPDIFF(SECOND, reserved_at, NOW()) >= threshold

-- @transaction off
SET NAMES utf8mb4;

ALTER TABLE `name_registry`
    ADD COLUMN `reserved_at` DATETIME NULL DEFAULT NULL;

DROP PROCEDURE IF EXISTS `USP_NAME_REGISTER`;

DROP PROCEDURE IF EXISTS `USP_NAME_RESERVE`;
DELIMITER $$
CREATE PROCEDURE `USP_NAME_RESERVE`(IN uname NVARCHAR(256), IN in_world INT UNSIGNED, IN threshold_sec INT UNSIGNED)
BEGIN
    DECLARE existing_id       INT UNSIGNED DEFAULT NULL;
    DECLARE existing_reserved DATETIME     DEFAULT NULL;
    DECLARE CONTINUE HANDLER FOR NOT FOUND SET existing_id = NULL;
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        SELECT 0 AS result, 0 AS uid;
    END;

    START TRANSACTION;

    SELECT `id`, `reserved_at` INTO existing_id, existing_reserved
    FROM `name_registry` WHERE `name` = uname FOR UPDATE;

    IF existing_id IS NULL THEN
        INSERT INTO `name_registry` (`name`, `world`, `reserved_at`) VALUES (uname, in_world, NOW());
        SELECT 1 AS result, LAST_INSERT_ID() AS uid;
    ELSEIF existing_reserved IS NULL THEN
        SELECT 0 AS result, 0 AS uid;
    ELSEIF TIMESTAMPDIFF(SECOND, existing_reserved, NOW()) >= threshold_sec THEN
        DELETE FROM `name_registry` WHERE `id` = existing_id;
        INSERT INTO `name_registry` (`name`, `world`, `reserved_at`) VALUES (uname, in_world, NOW());
        SELECT 1 AS result, LAST_INSERT_ID() AS uid;
    ELSE
        SELECT 0 AS result, 0 AS uid;
    END IF;

    COMMIT;
END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS `USP_NAME_KEEPALIVE`;
DELIMITER $$
CREATE PROCEDURE `USP_NAME_KEEPALIVE`(IN uname NVARCHAR(256))
BEGIN
    UPDATE `name_registry`
    SET `reserved_at` = NOW()
    WHERE `name` = uname AND `reserved_at` IS NOT NULL;
END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS `USP_NAME_CONFIRM`;
DELIMITER $$
CREATE PROCEDURE `USP_NAME_CONFIRM`(IN uname NVARCHAR(256))
BEGIN
    UPDATE `name_registry`
    SET `reserved_at` = NULL
    WHERE `name` = uname AND `reserved_at` IS NOT NULL;
END$$
DELIMITER ;
