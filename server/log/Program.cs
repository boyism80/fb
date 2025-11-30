using Dapper;
using Http.Extension;
using Http.Service;
using Log.Formatter;
using Log.Repository;
using Log.Service;
using Log.Worker;

Dapper.DefaultTypeMap.MatchNamesWithUnderscores = true;

var builder = WebApplication.CreateBuilder(args);
builder.Logging.ClearProviders();
builder.Logging.AddConsole();

// Add services to the container
builder.Services.AddControllers();
builder.Services.AddMvc(opt =>
{
    opt.InputFormatters.Insert(0, new OctetStreamInputFormatter());
});
builder.Services.AddEndpointsApiExplorer();
builder.Services.AddSwaggerGen();

// Register services
builder.Services.AddSingleton<RedisService>();
builder.Services.AddScoped<DbContext>();
builder.Services.AddScoped<LogService>();
builder.Services.AddScoped<LogRepository>();
builder.Services.AddHostedService<LogWriterService>();

var app = builder.Build();

// Configure the HTTP request pipeline
if (app.Environment.IsDevelopment())
{
    app.UseSwagger();
    app.UseSwaggerUI();
}

app.UseAuthorization();
app.MapControllers();

app.Run();
