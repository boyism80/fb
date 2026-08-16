-- Persist 6.51 visible-helmet (C2S 0x1B id 0x0E; same slot as 5.65 lock-walk-speed).
ALTER TABLE `option`
  ADD COLUMN `visible_helmet` tinyint unsigned NOT NULL DEFAULT 1 AFTER `pk_protect`;
