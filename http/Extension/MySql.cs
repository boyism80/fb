using Dapper;
using Newtonsoft.Json;
using System.Data;

namespace Http.Extension
{
    public static class MySql
    {
        public static string Escape<T>(this T obj)
        {
            if (obj == null)
                return "NULL";

            return obj switch
            {
                string s => s == null ? "NULL" : $"'{s}'",
                bool b => b ? "1" : "0",
                DateTime dt => $"'{dt:yyyy-MM-dd HH:mm:ss.ffffff}'",
                _ => obj.ToString(),
            };
        }
    }

    public class JsonTypeHandler : SqlMapper.ITypeHandler
    {
        public void SetValue(IDbDataParameter parameter, object value)
        {
            parameter.Value = JsonConvert.SerializeObject(value);
        }

        public object Parse(Type destinationType, object value)
        {
            return JsonConvert.DeserializeObject(value as string, destinationType);
        }
    }

}
