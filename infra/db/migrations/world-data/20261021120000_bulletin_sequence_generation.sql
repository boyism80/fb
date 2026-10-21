ALTER TABLE `bulletin_sequence`
  ADD COLUMN `generation` int unsigned NOT NULL DEFAULT 0 COMMENT 'Bumped on every renumber; article cache keys carry it so a renumbered id never reads an older article' AFTER `id`;
