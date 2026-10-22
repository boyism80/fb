-- @transaction off

DROP PROCEDURE IF EXISTS `USP_MAIL_WRITE`;
DELIMITER $$
CREATE PROCEDURE `USP_MAIL_WRITE`(
    IN user INT,
    IN sender INT,
    IN title NVARCHAR(64),
    IN contents NVARCHAR(256),
    IN max_count INT UNSIGNED
)
BEGIN
    DECLARE new_id INT UNSIGNED;
    DECLARE unread_count INT UNSIGNED DEFAULT 0;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        SELECT 0 AS RESULT, 0 AS UNREAD;
    END;

    START TRANSACTION;

    CALL USP_MAIL_NEXT_ID(user, max_count, new_id);
    IF new_id = 0 THEN
        ROLLBACK;
        SELECT 2 AS RESULT, 0 AS UNREAD;
    ELSE
        INSERT INTO mail (`id`, `user`, `sender`, `title`, `contents`)
        VALUES (new_id, user, sender, title, contents);

        SELECT COUNT(`id`) INTO unread_count FROM mail WHERE mail.`user` = user AND `read` = 0 AND deleted = 0;

        COMMIT;

        SELECT 1 AS RESULT, unread_count AS UNREAD;
        SELECT * FROM mail WHERE mail.`id` = new_id AND mail.`user` = user;
    END IF;
END$$
DELIMITER ;
