UPDATE `storage_box` s
JOIN `unsettled_request` u ON u.`user` = s.`user` AND u.`type` = 1 AND u.`target` = s.`id`
SET s.`received` = 0
WHERE s.`received` = 1 AND s.`claim_id` = u.`id`;

DELETE FROM `unsettled_request` WHERE `type` = 1;

DELETE u FROM `unsettled_request` u
JOIN `storage_box` s ON s.`user` = u.`user` AND s.`external_ref` = CONCAT('marketplace:list:', u.`id`)
WHERE u.`type` = 2;

DELETE u FROM `unsettled_request` u
JOIN `storage_box` s ON s.`user` = u.`user` AND s.`external_ref` = CONCAT('marketplace:purchase:', u.`id`)
WHERE u.`type` = 3;
