RENAME TABLE `mail_system_delivered` TO `mail_system_received`;

DROP PROCEDURE IF EXISTS `USP_MAIL_NEXT_ID`;
DELIMITER $$
CREATE PROCEDURE `USP_MAIL_NEXT_ID`(
    IN p_user INT UNSIGNED,
    IN p_max_count INT UNSIGNED,
    OUT p_id INT UNSIGNED
)
BEGIN
    DECLARE v_sequence INT UNSIGNED DEFAULT NULL;
    DECLARE v_count INT UNSIGNED DEFAULT 0;

    -- Keeps a missing sequence row from reaching the caller's cursor NOT FOUND handler.
    DECLARE CONTINUE HANDLER FOR NOT FOUND BEGIN END;

    -- The caller must run this inside its transaction; 0 means the mailbox is full.
    SET p_id = 0;

    SELECT id INTO v_sequence
    FROM mail_sequence
    WHERE mail_sequence.`user` = p_user FOR UPDATE;

    IF v_sequence IS NULL THEN
        SET p_id = 1;
        INSERT INTO mail_sequence (`user`, `id`) VALUES (p_user, p_id);
    ELSE
        SELECT COUNT(*) INTO v_count
        FROM mail
        WHERE mail.`user` = p_user AND mail.deleted = 0;

        IF v_count < p_max_count THEN
            IF v_sequence >= p_max_count THEN
                INSERT IGNORE INTO mail_system_received (`user`, `system_mail_id`)
                SELECT mail.`user`, mail.system_mail_id
                FROM mail
                WHERE mail.`user` = p_user AND mail.deleted = 1 AND mail.system_mail_id IS NOT NULL;

                DELETE FROM mail WHERE mail.`user` = p_user AND mail.deleted = 1;

                -- Moving every id above the range first keeps the renumbering from colliding on the primary key.
                UPDATE mail SET id = id + 1000000 WHERE mail.`user` = p_user;
                UPDATE mail m
                INNER JOIN (
                    SELECT mail.id, ROW_NUMBER() OVER (ORDER BY mail.id) AS rn
                    FROM mail
                    WHERE mail.`user` = p_user
                ) r ON m.`user` = p_user AND m.id = r.id
                SET m.id = r.rn;

                SET v_sequence = ROW_COUNT();
            END IF;

            SET p_id = v_sequence + 1;
            UPDATE mail_sequence SET id = p_id WHERE mail_sequence.`user` = p_user;
        END IF;
    END IF;
END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS `USP_MAIL_DELIVER_SYSTEM_MANY`;

DROP PROCEDURE IF EXISTS `USP_MAIL_SEND_SYSTEM_MANY`;
DELIMITER $$
CREATE PROCEDURE `USP_MAIL_SEND_SYSTEM_MANY`(
    IN system_mail_id INT UNSIGNED,
    IN sender INT,
    IN max_count INT UNSIGNED
)
BEGIN
    DECLARE done INT DEFAULT FALSE;
    DECLARE v_user INT UNSIGNED;
    DECLARE v_title NVARCHAR(64);
    DECLARE v_contents NVARCHAR(256);
    DECLARE new_id INT UNSIGNED;

    -- Caller must populate session temp tables on the same connection:
    --   tmp_mail_write_users (user_id)
    --   tmp_mail_send_content (user_id, title, contents) — per-recipient rendered text

    DECLARE user_cursor CURSOR FOR
        SELECT u.user_id, c.title, c.contents
        FROM tmp_mail_write_users u
        INNER JOIN tmp_mail_send_content c ON c.user_id = u.user_id
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

        -- A deleted system mail row, or its record after renumbering, counts as received.
        IF EXISTS (
            SELECT 1 FROM mail
            WHERE mail.`user` = v_user
              AND mail.system_mail_id = system_mail_id
        ) OR EXISTS (
            SELECT 1 FROM mail_system_received d
            WHERE d.`user` = v_user
              AND d.system_mail_id = system_mail_id
        ) THEN
            ITERATE read_loop;
        END IF;

        CALL USP_MAIL_NEXT_ID(v_user, max_count, new_id);
        SET done = FALSE;
        IF new_id = 0 THEN
            ITERATE read_loop;
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

DROP PROCEDURE IF EXISTS `USP_STORAGE_BOX_DELIVER_SYSTEM_MANY`;
