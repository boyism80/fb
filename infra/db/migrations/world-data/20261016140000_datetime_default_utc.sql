-- @transaction off
-- Column defaults and ON UPDATE CURRENT_TIMESTAMP follow the session clock.
-- Point them at UTC_TIMESTAMP() without changing the session time_zone.

DELIMITER $$

DROP PROCEDURE IF EXISTS fb_datetime_defaults_to_utc $$

CREATE PROCEDURE fb_datetime_defaults_to_utc()
BEGIN
    DECLARE done INT DEFAULT 0;
    DECLARE stmt TEXT;
    DECLARE cur CURSOR FOR
        SELECT CONCAT(
            'ALTER TABLE `', TABLE_NAME, '` MODIFY `', COLUMN_NAME, '` ',
            COLUMN_TYPE, ' ',
            IF(IS_NULLABLE = 'YES', 'NULL', 'NOT NULL'),
            ' DEFAULT (UTC_TIMESTAMP())',
            IF(COLUMN_COMMENT = '', '', CONCAT(' COMMENT ', QUOTE(COLUMN_COMMENT))))
        FROM information_schema.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE()
          AND DATA_TYPE = 'datetime'
          AND TABLE_NAME <> 'schema_migrations'
          AND (
                COLUMN_DEFAULT = 'CURRENT_TIMESTAMP'
                OR EXTRA LIKE '%on update CURRENT_TIMESTAMP%'
              );
    DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = 1;

    OPEN cur;
    read_loop: LOOP
        FETCH cur INTO stmt;
        IF done = 1 THEN
            LEAVE read_loop;
        END IF;
        SET @fb_dt_sql = stmt;
        PREPARE fb_dt_stmt FROM @fb_dt_sql;
        EXECUTE fb_dt_stmt;
        DEALLOCATE PREPARE fb_dt_stmt;
    END LOOP;
    CLOSE cur;
END $$

DELIMITER ;

CALL fb_datetime_defaults_to_utc();
DROP PROCEDURE IF EXISTS fb_datetime_defaults_to_utc;
