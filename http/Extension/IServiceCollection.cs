using http.Formatter;

namespace http.Extension
{
    public static class IServiceCollectionEx
    {
        public static void AddFlatBufferFormatter(this IServiceCollection services)
        {
            services.AddMvc(opt => 
            {
                opt.InputFormatters.Insert(0, new FlatBufferInputFormatter());
                opt.OutputFormatters.Insert(0, new FlatBufferOutputFormatter());
            });
        }
    }
}
