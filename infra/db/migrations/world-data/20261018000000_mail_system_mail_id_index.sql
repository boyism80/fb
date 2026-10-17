-- @transaction off
-- The system mail archive deletes by system_mail_id alone, which the (user, system_mail_id) keys cannot serve.

ALTER TABLE `mail`
  ADD INDEX `IX_MAIL_SYSTEM_MAIL_ID` (`system_mail_id`);

ALTER TABLE `mail_system_received`
  ADD INDEX `IX_MAIL_SYSTEM_RECEIVED_SYSTEM_MAIL_ID` (`system_mail_id`);
