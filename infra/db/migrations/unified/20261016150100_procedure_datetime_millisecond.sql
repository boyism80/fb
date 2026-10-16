-- Recreate writers so datetime(3) columns keep fractional seconds.
-- @transaction off

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
        INSERT INTO `name_registry` (`name`, `world`, `reserved_at`) VALUES (uname, in_world, UTC_TIMESTAMP(3));
        SELECT 1 AS result, LAST_INSERT_ID() AS uid;
    ELSEIF existing_reserved IS NULL THEN
        SELECT 0 AS result, 0 AS uid;
    ELSEIF TIMESTAMPDIFF(SECOND, existing_reserved, UTC_TIMESTAMP(3)) >= threshold_sec THEN
        UPDATE `name_registry` SET `reserved_at` = UTC_TIMESTAMP(3), `world` = in_world WHERE `id` = existing_id;
        SELECT 1 AS result, existing_id AS uid;
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
    SET `reserved_at` = UTC_TIMESTAMP(3)
    WHERE `name` = uname AND `reserved_at` IS NOT NULL;
END$$
DELIMITER ;
