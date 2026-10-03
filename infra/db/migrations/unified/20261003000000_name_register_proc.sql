-- Add USP_NAME_REGISTER: INSERT into name_registry with explicit id (allocated via Redis counter).
-- Character names are now reserved in Redis with TTL instead of DB, and committed here at
-- character creation time. This prevents ghost name_registry entries from failed sessions.

-- @transaction off
SET NAMES utf8mb4;

DROP PROCEDURE IF EXISTS `USP_NAME_REGISTER`;
DELIMITER $$
CREATE PROCEDURE `USP_NAME_REGISTER`(IN uid INT UNSIGNED, IN uname NVARCHAR(256), IN in_world INT UNSIGNED)
BEGIN
    INSERT INTO `name_registry` (`id`, `name`, `world`) VALUES (uid, uname, in_world);
END$$
DELIMITER ;
