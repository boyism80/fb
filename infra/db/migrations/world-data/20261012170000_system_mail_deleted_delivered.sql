-- A deleted system mail row still holds UX_USER_SYSTEM_MAIL, so the recipient counts as delivered.

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
        ) THEN
            ITERATE read_loop;
        END IF;

        SET new_id = NULL;
        SELECT id INTO new_id
        FROM mail_sequence
        WHERE mail_sequence.`user` = v_user FOR UPDATE;
        SET done = FALSE;

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
