-- @transaction off

DROP PROCEDURE IF EXISTS `USP_MAIL_NEXT_ID`;
DELIMITER $$
CREATE PROCEDURE `USP_MAIL_NEXT_ID`(
    IN p_user INT UNSIGNED,
    IN p_max_count INT UNSIGNED,
    OUT p_id INT UNSIGNED
)
BEGIN
    DECLARE v_sequence INT UNSIGNED;
    DECLARE v_count INT UNSIGNED DEFAULT 0;

    SET p_id = 0;

    INSERT INTO mail_sequence (`user`, `id`) VALUES (p_user, 0)
    ON DUPLICATE KEY UPDATE `id` = `id`;

    SELECT id INTO v_sequence
    FROM mail_sequence
    WHERE mail_sequence.`user` = p_user FOR UPDATE;

    SELECT COUNT(*) INTO v_count
    FROM mail
    WHERE mail.`user` = p_user AND mail.deleted = 0;

    IF v_count < p_max_count THEN
        IF v_sequence >= p_max_count THEN
            DELETE FROM mail WHERE mail.`user` = p_user AND mail.deleted = 1;

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
END$$
DELIMITER ;
