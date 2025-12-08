using AdminTool.Authorization;
using AdminTool.Options;
using Dapper;
using Http.Extension;
using Http.Service;
using Http.Worker;
using Microsoft.AspNetCore.Components.Authorization;
using Microsoft.AspNetCore.Components.Server.ProtectedBrowserStorage;

Dapper.DefaultTypeMap.MatchNamesWithUnderscores = true;
SqlMapper.AddTypeHandler(typeof(List<uint>), new JsonTypeHandler());
SqlMapper.AddTypeHandler(typeof(List<Http.Model.Buff>), new JsonTypeHandler());
SqlMapper.AddTypeHandler(typeof(List<Fb.Model.Dsl>), new JsonTypeHandler());
SqlMapper.AddTypeHandler(typeof(string), new UuidStringTypeHandler());

var builder = WebApplication.CreateBuilder(args);

// Add services to the container.
builder.Services.AddRazorPages();
builder.Services.AddServerSideBlazor();
builder.Services.AddScoped<ProtectedSessionStorage>();
builder.Services.AddScoped<AuthenticationStateProvider, AdminTool.Authentication.CustomAuthenticationStateProvider>();

// HTTP project services
builder.Services.AddSingleton<RedisService>();
builder.Services.AddSingleton<RedisDistributedLockService>();
builder.Services.AddSingleton<RabbitMqService>();
builder.Services.AddSingleton<LogService>();
builder.Services.AddSingleton<SessionService>();
builder.Services.AddSingleton<WriteBackService>();
builder.Services.AddScoped<DbContext>();
builder.Services.AddScoped<BanService>();
builder.Services.AddScoped<CacheService>();
builder.Services.AddSingleton<ServerStateService>();
builder.Services.AddSingleton<Http.Service.BulletinService>();
builder.Services.AddSingleton<Http.Service.BulletinCacheService>();
builder.Services.AddHostedService<Http.Service.BulletinBackgroundService>();
builder.Services.Configure<SecurityOptions>(builder.Configuration.GetSection("Security"));
builder.Services.AddSingleton<AdminTool.Services.SecurityService>();
builder.Services.AddScoped<StorageService>();
builder.Services.AddAuthorization();
builder.Services.AddSingleton<Microsoft.AspNetCore.Authorization.IAuthorizationHandler, MinimumRoleAuthorizationHandler>();
builder.Services.AddSingleton<Microsoft.AspNetCore.Authorization.IAuthorizationPolicyProvider, MinimumRolePolicyProvider>();

// Admin tool services
builder.Services.AddScoped<AdminTool.Services.AuthService>();
builder.Services.AddScoped<AdminTool.Services.UserService>();

var app = builder.Build();

// Load model data
var logger = app.Services.GetRequiredService<ILogger<DataTableLoader>>();
var dataTableLoader = new DataTableLoader(logger);
dataTableLoader.Run();

// Configure the HTTP request pipeline.
if (!app.Environment.IsDevelopment())
{
    app.UseExceptionHandler("/Error");
    app.UseHsts();
}

app.UseHttpsRedirection();
app.UseStaticFiles();
app.UseRouting();

app.MapBlazorHub();
app.MapFallbackToPage("/_Host");

app.Run();
