-- @transaction off
-- Store milliseconds on every datetime column. Writers must use UTC_TIMESTAMP(3)
-- or a fractional timestamp; assigning UTC_TIMESTAMP() into datetime(3) stores .000.

DELIMITER $$

DROP PROCEDURE IF EXISTS fb_datetime_to_millisecond $$

CREATE PROCEDURE fb_datetime_to_millisecond()
BEGIN
    DECLARE done INT DEFAULT 0;
    DECLARE stmt TEXT;
    DECLARE cur CURSOR FOR
        SELECT CONCAT(
            'ALTER TABLE `', TABLE_NAME, '` MODIFY `', COLUMN_NAME, '` datetime(3) ',
            IF(IS_NULLABLE = 'YES', 'NULL', 'NOT NULL'),
            CASE
                WHEN COLUMN_DEFAULT IS NULL THEN ''
                WHEN LOWER(REPLACE(COLUMN_DEFAULT, ' ', '')) IN
                     ('current_timestamp', 'current_timestamp()', 'utc_timestamp', 'utc_timestamp()')
                    THEN ' DEFAULT (UTC_TIMESTAMP(3))'
                ELSE CONCAT(' DEFAULT ', QUOTE(COLUMN_DEFAULT))
            END,
            IF(COLUMN_COMMENT = '', '', CONCAT(' COMMENT ', QUOTE(COLUMN_COMMENT))))
        FROM information_schema.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE()
          AND DATA_TYPE = 'datetime'
          AND DATETIME_PRECISION < 3
          AND TABLE_NAME <> 'schema_migrations';
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

CALL fb_datetime_to_millisecond();
DROP PROCEDURE IF EXISTS fb_datetime_to_millisecond;
