using Db.Service;
using Microsoft.AspNetCore.Mvc;
using Request = fb.protocol.db.request;
using Response = fb.protocol.db.response;

namespace db.Controllers
{
    [ApiController]
    [Route("account")]
    public class AccountController : ControllerBase
    {
        private readonly DbContext _dbContext;
        private readonly ILogger<AccountController> _logger;

        public AccountController(DbContext dbContext)
        {
            _dbContext = dbContext;
        }

        [HttpPost("create")]
        public async Task<Response.CreateCharacter> Create(Request.CreateCharacter request)
        {
            throw new NotImplementedException();
        }

        [HttpPost("init")]
        public async Task<Response.InitCharacter> Init(Request.InitCharacter request)
        {
            throw new NotImplementedException();
        }

        [HttpPost("finish")]
        public async Task<Response.FinishCharacter> Finish(Request.FinishCharacter request)
        {
            throw new NotImplementedException();
        }
    }
}