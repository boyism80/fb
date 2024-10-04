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
    [Route("user")]
    public class UserController : ControllerBase
    {
        private readonly IConfiguration _configuration;

        public UserController(IConfiguration configuration)
        {
            _configuration = configuration;
        }

        [HttpGet("uid/{name}")]
        public async Task<Response.GetUid> Uid(string name)
        {
            using var connection = new MySqlConnection(_configuration.GetConnectionString("Default"));
            var result = await connection.QueryAsync<uint>("USP_NAME_GET_ID", new 
            {
                n = "name"
            }, commandType: CommandType.StoredProcedure);

            if (result.Any())
            {
                return new Response.GetUid
                {
                    Uid = result.ElementAt(0),
                    Success = true
                };
            }
            else
            {
                return new Response.GetUid
                { 
                    Success = false
                };
            }
        }

        [HttpGet("account/{uid}")]
        public async Task<Response.Account> Account(uint uid)
        {
            using var connection = new MySqlConnection(_configuration.GetConnectionString("Default"));
            var user = await connection.QueryFirstOrDefaultAsync<User>($"SELECT * FROM user WHERE id = {uid} LIMIT 1");
            if (user == null)
            {
                return new Response.Account
                {
                    Success = false
                };
            }
            return new Response.Account
            { 
                Pw = user.Pw,
                Map = user.Map,
                Success = true
            };
        }

        [HttpPost("reserve-name")]
        public async Task<Response.ReserveName> ReserveName(Request.ReserveName request)
        {
            // 동시성 제어 필요
            using var connection = new MySqlConnection(_configuration.GetConnectionString("Default"));
            var result = await connection.QueryFirstAsync<ReserveNameResult>("USP_NAME_SET", new
            {
                name = "name"
            }, commandType: CommandType.StoredProcedure);

            return new Response.ReserveName
            { 
                Uid = result.Uid,
                Success = result.Result
            };
        }

        [HttpPost("init-ch")]
        public async Task<Response.InitCharacter> InitCharacter(Request.InitCharacter request)
        {
            using var connection = new MySqlConnection(_configuration.GetConnectionString("Default"));
            var success = await connection.QueryFirstAsync<bool>("USP_CHARACTER_INIT", new
            {
                id = request.Uid,
                uname = request.Name,
                pw = request.Pw,
                base_hp = request.Hp,
                base_mp = request.Mp,
                map = request.Map,
                position_x = request.X,
                position_y = request.Y,
                admin = request.Admin
            }, commandType: CommandType.StoredProcedure);

            return new Response.InitCharacter
            { 
                Success = success
            };
        }

        [HttpPost("mk-ch")]
        public async Task<Response.MakeCharacter> MakeCharacter(Request.MakeCharacter request)
        {
            using var connection = new MySqlConnection(_configuration.GetConnectionString("Default"));
            var affectedRows = await connection.ExecuteAsync("USP_CHARACTER_CREATE_FINISH", new
            {
                id = request.Uid,
                look = request.Hair,
                sex = request.Sex,
                nation = request.Nation,
                creature = request.Creature
            }, commandType: CommandType.StoredProcedure);

            return new Response.MakeCharacter
            { 
                Success = (affectedRows != 0)
            };
        }
    }
}