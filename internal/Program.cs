using http.Service;
using http.Worker;
using Internal.Formatter;
using Internal.Service;

namespace Internal;

public class Program
{
    public static void Main(string[] args)
    {
        var builder = WebApplication.CreateBuilder(args);

        // Add services to the container.

        builder.Services.AddControllers();
        builder.Services.AddMvc(opt => 
        {
            opt.InputFormatters.Insert(0, new FlatBufferInputFormatter());
            opt.OutputFormatters.Insert(0, new FlatBufferOutputFormatter());
        });
        // Learn more about configuring Swagger/OpenAPI at https://aka.ms/aspnetcore/swashbuckle
        builder.Services.AddEndpointsApiExplorer();
        builder.Services.AddSwaggerGen();
        builder.Services.AddSingleton<RedisService>();
        builder.Services.AddSingleton<Fb.Model.Model>();
        builder.Services.AddSingleton<RabbitMqService>();
        builder.Services.AddSingleton<SessionService>();

        var app = builder.Build();
        var dataTableLoader = ActivatorUtilities.CreateInstance(app.Services.CreateScope().ServiceProvider, typeof(DataTableLoader)) as DataTableLoader;
        dataTableLoader.Run();

        // Configure the HTTP request pipeline.
        if (app.Environment.IsDevelopment())
        {
            app.UseSwagger();
            app.UseSwaggerUI();
        }

        app.UseAuthorization();


        app.MapControllers();

        app.Run();
    }
}
