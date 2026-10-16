-- Stored datetimes are Korean civil time. Shift them to UTC once.
-- marketplace_listing.expire_date is already written from UTC.

DELIMITER $$

DROP PROCEDURE IF EXISTS fb_shift_kst_datetimes_to_utc $$

CREATE PROCEDURE fb_shift_kst_datetimes_to_utc()
BEGIN
    DECLARE done INT DEFAULT 0;
    DECLARE stmt TEXT;
    DECLARE cur CURSOR FOR
        SELECT CONCAT(
            'UPDATE `', TABLE_NAME, '` SET `', COLUMN_NAME,
            '` = DATE_SUB(`', COLUMN_NAME, '`, INTERVAL 9 HOUR) WHERE `',
            COLUMN_NAME, '` > ''1000-01-01 00:00:00''')
        FROM information_schema.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE()
          AND DATA_TYPE = 'datetime'
          AND TABLE_NAME <> 'schema_migrations'
          AND NOT (TABLE_NAME = 'marketplace_listing' AND COLUMN_NAME = 'expire_date');
    DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = 1;
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        RESIGNAL;
    END;

    START TRANSACTION;
    OPEN cur;
    read_loop: LOOP
        FETCH cur INTO stmt;
        IF done = 1 THEN
            LEAVE read_loop;
        END IF;
        SET @fb_shift_sql = stmt;
        PREPARE fb_shift_stmt FROM @fb_shift_sql;
        EXECUTE fb_shift_stmt;
        DEALLOCATE PREPARE fb_shift_stmt;
    END LOOP;
    CLOSE cur;
    COMMIT;
END $$

DELIMITER ;

CALL fb_shift_kst_datetimes_to_utc();
DROP PROCEDURE IF EXISTS fb_shift_kst_datetimes_to_utc;
