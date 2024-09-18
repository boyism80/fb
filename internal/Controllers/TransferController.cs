using Microsoft.AspNetCore.Mvc;
using request = fb.protocol.inter.request;
using response = fb.protocol.inter.response;

namespace Internal.Controllers;
[ApiController]
[Route("[controller]")]
public class TransferController : ControllerBase
{
    private readonly ILogger<TransferController> _logger;

    public TransferController(ILogger<TransferController> logger)
    {
        _logger = logger;
    }

    [HttpPost]
    public async Task<response.Transfer> Transfer(request.Transfer request)
    {
        return new response.Transfer
        {
            Name = request.Name,
            Code = response.TransferResultCode.CONNECTED,
            Fd = request.Fd,
            Ip = "127.0.0.1",
            Port = 2001,
            Map = request.Map,
            X = request.X,
            Y = request.Y
        };
    }
}
