using Fb.Model;
using Microsoft.Extensions.Logging;
using Response = fb.protocol._internal.response;

namespace Http.Service.Amqp
{
    [AmqpHandler("fb.global")]
    public sealed class ReloadTablesHandler : AmqpHandler<Response.ReloadTables>
    {
        private readonly ILogger<ReloadTablesHandler> _logger;

        public ReloadTablesHandler(ILogger<ReloadTablesHandler> logger)
        {
            _logger = logger;
        }

        protected override async Task HandleAsync(Response.ReloadTables message, CancellationToken cancellationToken)
        {
            if (message.Error != 0)
            {
                _logger.LogWarning("ReloadTables ignored due to error code {Error}", message.Error);
                return;
            }

            try
            {
                _logger.LogInformation("ReloadTables received; reloading data tables");
                await Table.ReloadAsync(cancellationToken);
                _logger.LogInformation("Data tables reloaded");
            }
            catch (Exception ex)
            {
                // Hosts without loaded tables (e.g. write-back) still receive fb.global.
                _logger.LogError(ex, "Data table reload failed");
            }
        }
    }
}
