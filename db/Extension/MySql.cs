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
                string s => string.IsNullOrEmpty(s) ? "NULL" : $"'{s}'",
                bool b => b ? "1" : "0",
                DateTime dt => $"'{dt:yyyy-MM-dd HH:mm:ss.ffffff}'",
                _ => obj.ToString(),
            };
        }
    }
}
