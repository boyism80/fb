-- @transaction off

DROP PROCEDURE IF EXISTS `USP_ASSERT_LISTINGS_ESCROWED`;
DELIMITER $$
CREATE PROCEDURE `USP_ASSERT_LISTINGS_ESCROWED`()
BEGIN
    IF EXISTS (
        SELECT 1 FROM `unsettled_request` u
        WHERE u.`type` = 2
          AND NOT EXISTS (SELECT 1 FROM `escrow` e WHERE e.`user` = u.`user` AND e.`id` = u.`id`)
          AND NOT EXISTS (SELECT 1 FROM `item` i WHERE i.`owner` = u.`user` AND i.`escrow_id` = u.`id` AND i.`deleted` = 0)
    ) THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'unsettled_request has marketplace listings made before item escrow; settle them with the previous build before this deploy';
    END IF;
END$$
DELIMITER ;

CALL `USP_ASSERT_LISTINGS_ESCROWED`();
DROP PROCEDURE IF EXISTS `USP_ASSERT_LISTINGS_ESCROWED`;
