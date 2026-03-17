namespace Http.Util
{
    public static class DateTimeEx
    {
        public static readonly System.DateTime _epoch = new System.DateTime(1996, 3, 31, 0, 0, 0, DateTimeKind.Local);

        public static string ToBaramTime(this System.DateTime now)
        {
            var diff = now - _epoch;
            var totalMs = (long)diff.TotalMilliseconds;

            // Convert to Baram time units
            // 1 Baram day = 3 real hours = 10800000 milliseconds
            var baramDays = totalMs / 10800000;
            var baramYear = (int)(baramDays / 365) + 1;
            var remainDays = (int)(baramDays % 365);
            var baramMonth = Math.Min(remainDays / 30 + 1, 12);

            var season = baramMonth switch
            {
                12 or 1 or 2 => "寃⑥슱",
                3 or 4 or 5 => "遊?,
                6 or 7 or 8 => "?щ쫫",
                _ => "媛??  // 9, 10, 11
            };

            return $"諛붾엺??{baramYear}??{season}";
        }
    }
}
