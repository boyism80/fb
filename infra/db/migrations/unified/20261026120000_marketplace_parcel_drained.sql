-- @transaction off

DROP PROCEDURE IF EXISTS `USP_ASSERT_PARCELS_SENT`;
DELIMITER $$
CREATE PROCEDURE `USP_ASSERT_PARCELS_SENT`()
BEGIN
    IF EXISTS (SELECT 1 FROM `marketplace_parcel` WHERE `sent_date` IS NULL) THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'marketplace_parcel has unsent rows; let the old marketplace send them before this deploy';
    END IF;
END$$
DELIMITER ;

CALL `USP_ASSERT_PARCELS_SENT`();
DROP PROCEDURE IF EXISTS `USP_ASSERT_PARCELS_SENT`;
