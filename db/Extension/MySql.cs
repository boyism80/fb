namespace Db.Extension
{
    public static class MySql
    {
        public static string Escape<T>(this T obj)
        {
            if (obj == null)
                return "NULL";

            return obj switch
            {
                string s => $"'{s}'",
                _ => obj.ToString(),
            };
        }
    }
}
