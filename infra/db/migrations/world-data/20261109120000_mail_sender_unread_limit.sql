-- @transaction off

ALTER TABLE `mail` ADD INDEX `IX_USER_SENDER` (`user`, `sender`);

DROP PROCEDURE IF EXISTS `USP_MAIL_WRITE`;
DELIMITER $$
CREATE PROCEDURE `USP_MAIL_WRITE`(
    IN user INT,
    IN sender INT,
    IN title NVARCHAR(64),
    IN contents NVARCHAR(256),
    IN max_count INT UNSIGNED,
    IN sender_unread_limit INT UNSIGNED
)
BEGIN
    DECLARE new_id INT UNSIGNED;
    DECLARE unread_count INT UNSIGNED DEFAULT 0;
    DECLARE sender_unread INT UNSIGNED DEFAULT 0;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        SELECT 0 AS RESULT, 0 AS UNREAD;
    END;

    START TRANSACTION;

    CALL USP_MAIL_NEXT_ID(user, max_count, new_id);
    SELECT COUNT(`id`) INTO sender_unread FROM mail WHERE mail.`user` = user AND mail.`sender` = sender AND `read` = 0 AND deleted = 0;

    IF new_id = 0 THEN
        ROLLBACK;
        SELECT 2 AS RESULT, 0 AS UNREAD;
    ELSEIF sender_unread >= sender_unread_limit THEN
        ROLLBACK;
        SELECT 3 AS RESULT, 0 AS UNREAD;
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
