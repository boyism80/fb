ALTER TABLE `user`
  ADD COLUMN `evaluation_state` json DEFAULT NULL COMMENT 'Evaluation playtime and recently evaluated targets' AFTER `evaluation`;
