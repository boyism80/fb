using AdminTool.Authorization;
using AdminTool.Options;
using Dapper;
using Http.Extension;
using Http.Service;
using Http.Service.Amqp;
using Http.Worker;
using Microsoft.AspNetCore.Components.Authorization;
using Microsoft.AspNetCore.Components.Server.ProtectedBrowserStorage;

Dapper.DefaultTypeMap.MatchNamesWithUnderscores = true;
SqlMapper.AddTypeHandler(typeof(List<uint>), new JsonTypeHandler());
SqlMapper.AddTypeHandler(typeof(List<Http.Model.Buff>), new JsonTypeHandler());
SqlMapper.AddTypeHandler(typeof(List<Fb.Model.Dsl>), new JsonTypeHandler());
SqlMapper.AddTypeHandler(typeof(Dictionary<string, List<Fb.Model.Dsl>>), new JsonTypeHandler());
SqlMapper.AddTypeHandler(typeof(Http.Model.Mimicry), new JsonTypeHandler());

var builder = WebApplication.CreateBuilder(args);

// Local overrides (gitignored). Optional so CI/k8s can rely on env vars instead.
builder.Configuration.AddJsonFile("appsettings.Local.json", optional: true, reloadOnChange: true);

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
builder.Services.AddHttpAutoMapper();
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
builder.Services.Configure<AdminTool.Options.TablePublishOptions>(
    builder.Configuration.GetSection(AdminTool.Options.TablePublishOptions.SectionName));
builder.Services.Configure<AdminTool.Options.ScriptPublishOptions>(
    builder.Configuration.GetSection(AdminTool.Options.ScriptPublishOptions.SectionName));
builder.Services.AddHttpClient(nameof(AdminTool.Services.TablePublishService));
builder.Services.AddHttpClient(nameof(AdminTool.Services.ScriptPublishService));
builder.Services.AddSingleton<AdminTool.Services.SecurityService>();
builder.Services.AddScoped<StorageService>();
builder.Services.AddAuthorization();
builder.Services.AddSingleton<Microsoft.AspNetCore.Authorization.IAuthorizationHandler, MinimumRoleAuthorizationHandler>();
builder.Services.AddSingleton<Microsoft.AspNetCore.Authorization.IAuthorizationPolicyProvider, MinimumRolePolicyProvider>();

// Admin tool services
builder.Services.AddScoped<AdminTool.Services.ThemeService>();
builder.Services.AddScoped<AdminTool.Services.AuthService>();
builder.Services.AddScoped<AdminTool.Services.UserService>();
builder.Services.AddScoped<AdminTool.Services.UserDetailService>();
builder.Services.AddScoped<AdminTool.Services.MarketplaceAdminService>();
builder.Services.AddScoped<AdminTool.Services.GameDbResetService>();
builder.Services.AddSingleton<AdminTool.Services.AdminActivityLogService>();
builder.Services.AddSingleton<AdminTool.Services.TablePublishService>();
builder.Services.AddSingleton<AdminTool.Services.ScriptPublishService>();
builder.Services.AddSingleton<Http.Service.MaintenanceService>();
builder.Services.AddAmqpListener<ReloadTablesHandler>();

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
    app.UseHttpsRedirection();
}
app.UseStaticFiles();
app.UseRouting();

app.UseAuthorization();

app.MapRazorPages();
app.MapBlazorHub();
app.MapFallbackToPage("/_Host");

app.Run();
