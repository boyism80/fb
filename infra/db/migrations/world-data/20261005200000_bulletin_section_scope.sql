DROP PROCEDURE IF EXISTS `USP_BULLETIN_DELETE`;
DELIMITER $$
CREATE PROCEDURE `USP_BULLETIN_DELETE`(IN section INT, IN id INT, IN user INT, IN ignore_owner TINYINT)
BEGIN
    DECLARE _deleted TINYINT;
    DECLARE _owner INT;

    SELECT bulletin.deleted, bulletin.user
    INTO _deleted, _owner
    FROM bulletin
    WHERE bulletin.id = id AND bulletin.section = section;

    IF _deleted IS NULL THEN
        SELECT -1 AS result;
    ELSEIF _deleted = 1 THEN
        SELECT -2 AS result;
    ELSEIF _owner != user AND ignore_owner = 0 THEN
        SELECT -3 AS result;
    ELSE
        IF ignore_owner = 1 THEN
            UPDATE bulletin SET deleted = 1, updated_date = NOW()
            WHERE bulletin.id = id AND bulletin.section = section;
        ELSE
            UPDATE bulletin SET deleted = 1, updated_date = NOW()
            WHERE bulletin.id = id AND bulletin.section = section AND bulletin.user = user;
        END IF;
        SELECT 1 AS result;
    END IF;
END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS `USP_BULLETIN_UPDATE`;
DELIMITER $$
CREATE PROCEDURE `USP_BULLETIN_UPDATE`(
    IN p_section INT,
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
    WHERE bulletin.id = p_id AND bulletin.section = p_section;

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
            WHERE bulletin.id = p_id AND bulletin.section = p_section;
        ELSE
            UPDATE bulletin
            SET title = p_title,
                contents = p_contents,
                updated_date = NOW()
            WHERE bulletin.id = p_id AND bulletin.section = p_section AND bulletin.user = p_user;
        END IF;

        IF ROW_COUNT() = 0 THEN
            SELECT -4 AS result;
        ELSE
            SELECT 1 AS result;
        END IF;
    END IF;
END$$
DELIMITER ;
