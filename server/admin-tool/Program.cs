using AdminTool.Authorization;
using AdminTool.Options;
using AdminTool.Services;
using Dapper;
using Http.Extension;
using Http.Service;
using Http.Worker;
using Microsoft.AspNetCore.Components.Authorization;
using Microsoft.AspNetCore.Components.Server.ProtectedBrowserStorage;

Dapper.DefaultTypeMap.MatchNamesWithUnderscores = true;
SqlMapper.AddTypeHandler(typeof(List<uint>), new JsonTypeHandler());
SqlMapper.AddTypeHandler(typeof(List<Http.Model.Buff>), new JsonTypeHandler());

var builder = WebApplication.CreateBuilder(args);

// Add services to the container.
builder.Services.AddRazorPages();
builder.Services.AddServerSideBlazor();
builder.Services.AddScoped<ProtectedSessionStorage>();
builder.Services.AddScoped<AuthenticationStateProvider, AdminTool.Authentication.CustomAuthenticationStateProvider>();

// HTTP project services
builder.Services.AddSingleton<RedisService>();
builder.Services.AddSingleton<RedisDistributedLockService>();
builder.Services.AddSingleton<Fb.Model.Model>();
builder.Services.AddSingleton<RabbitMqService>();
builder.Services.AddSingleton<WriteBackService>();
builder.Services.AddScoped<DbContext>();
builder.Services.AddScoped<BanService>();
builder.Services.AddScoped<CacheService>();
builder.Services.AddSingleton<ServerStateService>();
builder.Services.Configure<SecurityOptions>(builder.Configuration.GetSection("Security"));
builder.Services.AddSingleton<SecurityService>();
builder.Services.AddAuthorization();
builder.Services.AddSingleton<Microsoft.AspNetCore.Authorization.IAuthorizationHandler, MinimumRoleAuthorizationHandler>();
builder.Services.AddSingleton<Microsoft.AspNetCore.Authorization.IAuthorizationPolicyProvider, MinimumRolePolicyProvider>();

// Admin tool services
builder.Services.AddScoped<AdminTool.Services.AuthService>();
builder.Services.AddScoped<AdminTool.Services.UserService>();

var app = builder.Build();

// Load model data
var dataTableLoader = ActivatorUtilities.CreateInstance(app.Services.CreateScope().ServiceProvider, typeof(DataTableLoader)) as DataTableLoader;
if (dataTableLoader != null)
{
    dataTableLoader.Run();
}

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
