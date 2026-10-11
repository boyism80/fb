-- Batch writers share one NOT FOUND handler between the recipient cursor and the
-- per-recipient sequence lookup. A recipient without a sequence row set `done` and
-- ended the loop early, and left `new_id` holding the previous recipient's value.

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

DROP PROCEDURE IF EXISTS `USP_STORAGE_BOX_DELIVER_SYSTEM_MANY`;
DELIMITER $$
CREATE PROCEDURE `USP_STORAGE_BOX_DELIVER_SYSTEM_MANY`(
    IN p_system_storage_box_id INT UNSIGNED,
    IN p_title NVARCHAR(128),
    IN p_message NVARCHAR(256),
    IN p_attachments JSON,
    IN p_expired_date DATETIME
)
BEGIN
    DECLARE done INT DEFAULT FALSE;
    DECLARE v_user INT UNSIGNED;
    DECLARE new_id INT UNSIGNED;

    -- Caller must populate tmp_storage_box_write_users (user_id) on the same connection.

    DECLARE user_cursor CURSOR FOR
        SELECT user_id
        FROM tmp_storage_box_write_users
        WHERE user_id > 0
        ORDER BY user_id;

    DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = TRUE;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        SELECT 0 AS RESULT;
    END;

    DROP TEMPORARY TABLE IF EXISTS tmp_storage_box_written;
    CREATE TEMPORARY TABLE tmp_storage_box_written (
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

        IF EXISTS (
            SELECT 1 FROM storage_box
            WHERE storage_box.`user` = v_user
              AND storage_box.system_storage_box_id = p_system_storage_box_id
              AND storage_box.deleted = 0
        ) THEN
            ITERATE read_loop;
        END IF;

        SET new_id = NULL;
        SELECT id INTO new_id
        FROM storage_box_sequence
        WHERE storage_box_sequence.`user` = v_user FOR UPDATE;
        SET done = FALSE;

        IF new_id IS NULL THEN
            SET new_id = 1;
            INSERT INTO storage_box_sequence (`user`, `id`) VALUES (v_user, new_id);
        ELSE
            SET new_id = new_id + 1;
            UPDATE storage_box_sequence SET id = new_id WHERE storage_box_sequence.`user` = v_user;
        END IF;

        INSERT INTO storage_box (
            `user`, `id`, `system_storage_box_id`, `title`, `message`,
            `attachments`, `received`, `expired_date`, `deleted`, `created_date`, `updated_date`
        ) VALUES (
            v_user, new_id, p_system_storage_box_id, p_title, p_message,
            p_attachments, 0, p_expired_date, 0, NOW(), NOW()
        );

        INSERT INTO tmp_storage_box_written (`user`, `id`) VALUES (v_user, new_id);
    END LOOP;

    CLOSE user_cursor;

    COMMIT;

    SELECT 1 AS RESULT;

    SELECT s.*
    FROM storage_box s
    INNER JOIN tmp_storage_box_written w ON w.`user` = s.`user` AND w.`id` = s.`id`;
END$$
DELIMITER ;
