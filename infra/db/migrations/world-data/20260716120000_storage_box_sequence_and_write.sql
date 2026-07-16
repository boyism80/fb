-- Personal storage_box sequence + write/deliver/claim helpers (mail-style).

CREATE TABLE IF NOT EXISTS `storage_box_sequence` (
  `user` int unsigned NOT NULL,
  `id` int unsigned NOT NULL,
  PRIMARY KEY (`user`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;

DROP PROCEDURE IF EXISTS `USP_STORAGE_BOX_WRITE`;
DELIMITER $$
CREATE PROCEDURE `USP_STORAGE_BOX_WRITE`(
    IN p_user INT UNSIGNED,
    IN p_system_storage_box_id INT UNSIGNED,
    IN p_title NVARCHAR(128),
    IN p_message NVARCHAR(256),
    IN p_attachments JSON,
    IN p_expired_date DATETIME
)
BEGIN
    DECLARE new_id INT UNSIGNED;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        SELECT 0 AS RESULT;
    END;

    START TRANSACTION;

    IF p_system_storage_box_id IS NOT NULL AND EXISTS (
        SELECT 1 FROM storage_box
        WHERE storage_box.`user` = p_user
          AND storage_box.system_storage_box_id = p_system_storage_box_id
          AND storage_box.deleted = 0
    ) THEN
        COMMIT;
        SELECT 1 AS RESULT;
        SELECT * FROM storage_box
        WHERE storage_box.`user` = p_user
          AND storage_box.system_storage_box_id = p_system_storage_box_id
          AND storage_box.deleted = 0
        LIMIT 1;
    ELSE
        SELECT id INTO new_id
        FROM storage_box_sequence
        WHERE storage_box_sequence.`user` = p_user FOR UPDATE;

        IF new_id IS NULL THEN
            SET new_id = 1;
            INSERT INTO storage_box_sequence (`user`, `id`) VALUES (p_user, new_id);
        ELSE
            SET new_id = new_id + 1;
            UPDATE storage_box_sequence SET id = new_id WHERE storage_box_sequence.`user` = p_user;
        END IF;

        INSERT INTO storage_box (
            `user`, `id`, `system_storage_box_id`, `title`, `message`,
            `attachments`, `received`, `expired_date`, `deleted`, `created_date`, `updated_date`
        ) VALUES (
            p_user, new_id, p_system_storage_box_id, p_title, p_message,
            p_attachments, 0, p_expired_date, 0, NOW(), NOW()
        );

        COMMIT;

        SELECT 1 AS RESULT;
        SELECT * FROM storage_box
        WHERE storage_box.`user` = p_user AND storage_box.`id` = new_id;
    END IF;
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

        SELECT id INTO new_id
        FROM storage_box_sequence
        WHERE storage_box_sequence.`user` = v_user FOR UPDATE;

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
