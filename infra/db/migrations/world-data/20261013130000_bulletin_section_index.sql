-- Renumbering a section deletes and updates by `section`; without this index those statements lock every row of
-- the table and block the other sections.
ALTER TABLE `bulletin` ADD KEY `IX_SECTION_ID` (`section`, `id`);
