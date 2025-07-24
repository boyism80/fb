namespace Http.Util
{
    /// <summary>
    /// Provides extension methods for DateTime operations including Baram calendar calculations
    /// </summary>
    public static class DateTimeEx
    {
        /// <summary>
        /// The epoch time for Baram calendar (1996-03-31 00:00:00)
        /// </summary>
        public static readonly System.DateTime _epoch = new System.DateTime(1996, 3, 31, 0, 0, 0, DateTimeKind.Local);

        /// <summary>
        /// Converts a DateTime to Baram calendar format string
        /// </summary>
        /// <param name="now">The DateTime to convert to Baram calendar time</param>
        /// <returns>String representing the Baram calendar time in format "바람력 N년 계절"</returns>
        /// <remarks>
        /// Baram calendar starts from 1996-03-31 00:00:00.
        /// 1 Baram day equals 3 real-world hours.
        /// 1 Baram year is 365 Baram days.
        /// Seasons are determined by Baram months:
        /// - Winter: 12, 1, 2
        /// - Spring: 3, 4, 5
        /// - Summer: 6, 7, 8
        /// - Fall: 9, 10, 11
        /// </remarks>
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
                12 or 1 or 2 => "겨울",
                3 or 4 or 5 => "봄",
                6 or 7 or 8 => "여름",
                _ => "가을"  // 9, 10, 11
            };

            return $"바람력 {baramYear}년 {season}";
        }
    }
}