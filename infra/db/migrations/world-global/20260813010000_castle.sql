CREATE TABLE `castle` (
  `divine_beast` tinyint unsigned NOT NULL COMMENT 'fb::model::DIVINE_BEAST enum value (0=VERMILION_BIRD,1=WHITE_TIGER,2=BLACK_TORTOISE,3=AZURE_DRAGON)',
  `owner_clan` int unsigned DEFAULT NULL,
  `created_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`divine_beast`),
  KEY `castle_owner_clan_idx` (`owner_clan`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;

INSERT INTO `castle` (`divine_beast`, `owner_clan`) VALUES (0, NULL), (1, NULL), (2, NULL), (3, NULL);
-- No FK to clan: clan rows live in sharded world-data; ownership is validated in CastleController.
