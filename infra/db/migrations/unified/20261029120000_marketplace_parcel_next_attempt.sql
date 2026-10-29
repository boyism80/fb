-- A failed parcel waits until next_attempt_date so rows of a failing world do not block the head of the queue.
ALTER TABLE `marketplace_parcel`
  ADD COLUMN `next_attempt_date` DATETIME(3) NULL DEFAULT NULL COMMENT 'Earliest retry time after a failed send; NULL=send now' AFTER `last_error`,
  DROP INDEX `IX_MARKETPLACE_PARCEL_PENDING`,
  ADD KEY `IX_MARKETPLACE_PARCEL_PENDING` (`sent_date`, `next_attempt_date`, `id`);
