ALTER TABLE `unsettled_request`
  MODIFY COLUMN `type` tinyint unsigned NOT NULL COMMENT '0: clan money, 1: storage claim, 2: marketplace list, 3: marketplace purchase, 4: clan fee';
