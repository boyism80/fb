CREATE TABLE `collection_unlock` (
  `user` int NOT NULL,
  `mob_id` int NOT NULL,
  `deleted` tinyint NOT NULL DEFAULT 0,
  `created_date` datetime NOT NULL,
  `updated_date` datetime NOT NULL,
  PRIMARY KEY (`user`, `mob_id`)
) ENGINE=InnoDB DEFAULT CHARSET=euckr;
