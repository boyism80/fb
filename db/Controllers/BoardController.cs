using Dapper;
using db.Model;
using Microsoft.AspNetCore.Mvc;
using MySqlConnector;
using System.Data;
using Request = fb.protocol.db.request;
using Response = fb.protocol.db.response;

namespace db.Controllers
{
    [ApiController]
    [Route("board")]
    public class BoardController : ControllerBase
    {
        private readonly IConfiguration _configuration;

        public BoardController(IConfiguration configuration)
        {
            _configuration = configuration;
        }

        [HttpGet("{section}")]
        public async Task<Response.GetArticleList> GetArticleList(uint section, [FromQuery(Name = "offset")] ushort offset)
        {
            throw new NotImplementedException();
        }

        [HttpGet("{section}/{article}")]
        public async Task<Response.GetArticle> GetArticle(uint section, ushort article)
        {
            throw new NotImplementedException();
        }

        [HttpPost("write")]
        public async Task<Response.WriteArticle> Write(Request.WriteArticle request)
        {
            throw new NotImplementedException();
        }

        [HttpPost("delete")]
        public async Task<Response.DeleteArticle> Delete(Request.DeleteArticle request)
        {
            throw new NotImplementedException();
        }
    }
}