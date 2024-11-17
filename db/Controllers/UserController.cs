using AutoMapper;
using Dapper;
using db.Model;
using Db.Model;
using Db.Service;
using Microsoft.AspNetCore.Mvc;
using System.Data;
using System.Security.Cryptography;
using System.Text;
using Request = fb.protocol.db.request;
using Response = fb.protocol.db.response;

namespace db.Controllers
{
    [ApiController]
    [Route("user")]
    public class UserController : ControllerBase
    {
        private readonly IConfiguration _configuration;
        private readonly IMapper _mapper;
        private readonly DbContext _dbContext;

        public UserController(IConfiguration configuration,
            IMapper mapper,
            DbContext dbContext)
        {
            _configuration = configuration;
            _mapper = mapper;
            _dbContext = dbContext;
        }

        [HttpGet("uid/{name}")]
        public async Task<Response.GetUid> Uid(string name)
        {
            await using var connection = _dbContext.Connection(-1);
            var result = await connection.QueryAsync<uint>("USP_NAME_GET_ID", new
            {
                n = name
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

        private static string SHA256Hash(string value)
        {
            var sha = new SHA256Managed();
            var hash = sha.ComputeHash(Encoding.ASCII.GetBytes(value));
            var builder = new StringBuilder();
            foreach (byte b in hash)
            {
                builder.AppendFormat("{0:x2}", b);
            }
            return builder.ToString();
        }

        [HttpPost("authenticate")]
        public async Task<Response.Authenticate> Authenticate(Request.Authenticate request)
        {
            await using var connection = _dbContext.Connection(request.Uid);
            var account = await connection.QueryFirstOrDefaultAsync<Account>($"SELECT id, pw, map FROM user WHERE id = {request.Uid} LIMIT 1");
            if (account == null)
            {
                return new Response.Authenticate
                {
                    ErrorCode = 1
                };
            }

            if (account.Pw != SHA256Hash(request.Pw))
            {
                return new Response.Authenticate
                {
                    ErrorCode = 2
                };
            }

            return new Response.Authenticate
            {
                ErrorCode = 0,
                Map = account.Map
            };
        }

        [HttpPost("reserve-name")]
        public async Task<Response.ReserveName> ReserveName(Request.ReserveName request)
        {
            // 동시성 제어 필요
            await using var connection = _dbContext.Connection(-1);
            var result = await connection.QueryFirstAsync<ReserveNameResult>("USP_NAME_SET", new
            {
                name = request.Name
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
            await using var connection = _dbContext.Connection(request.Uid);
            var success = await connection.QueryFirstAsync<bool>("USP_CHARACTER_INIT", new
            {
                id = request.Uid,
                uname = request.Name,
                pw = SHA256Hash(request.Pw),
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
            await using var connection = _dbContext.Connection(request.Uid);
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

        [HttpPost("change-pw")]
        public async Task<Response.ChangePw> ChangePassword(Request.ChangePw request)
        {
            await using var connection = _dbContext.Connection(request.Uid);
            using (var reader = await connection.ExecuteReaderAsync($"SELECT pw, birth FROM user WHERE id = {request.Uid}"))
            {
                if (await reader.ReadAsync() == false)
                {
                    return new Response.ChangePw
                    {
                        ErrorCode = 1
                    };
                }

                var pw = reader.GetString("pw");
                if (pw != SHA256Hash(request.Before))
                {
                    return new Response.ChangePw
                    {
                        ErrorCode = 2
                    };
                }

                var birth = reader.GetValue("birth");
                if ((birth is DBNull) || ((uint)birth != request.Birthday))
                {
                    return new Response.ChangePw
                    {
                        ErrorCode = 3
                    };
                }
            }

            await connection.ExecuteAsync($"UPDATE user SET pw = @pw WHERE id = @id LIMIT 1", new
            {
                id = request.Uid,
                pw = SHA256Hash(request.After)
            });

            return new Response.ChangePw
            {
                ErrorCode = 0
            };
        }

        [HttpGet("login/{uid}")]
        public async Task<Response.Login> Login(uint uid)
        {
            var ch = await _dbContext.Character.Get(uid);
            var items = await _dbContext.Item.Get(uid);
            var spells = await _dbContext.Spell.Get(uid);

            var response = new Response.Login
            {
                Character = _mapper.Map<fb.protocol.db.Character>(ch),
                Items = items.Select(_mapper.Map<fb.protocol.db.Item>).ToList(),
                Spells = spells.Select(_mapper.Map<fb.protocol.db.Spell>).ToList()
            };

            return response;
        }

        [HttpPost("save")]
        public async Task<Response.Save> Save(Request.Save request)
        {
            var ch = _mapper.Map<Character>(request.Character);
            await _dbContext.Character.Set(ch);

            var items = await _dbContext.Item.Get(request.Character.Id);
            foreach (var item in items)
            {
                item.Deleted = true;
            }
            await _dbContext.Item.Set(items.ToArray());
            items = _mapper.Map<fb.protocol.db.Item[], Item[]>(request.Items.ToArray());
            await _dbContext.Item.Set(items.ToArray());

            var spells = await _dbContext.Spell.Get(request.Character.Id);
            foreach (var item in spells)
            {
                item.Deleted = true;
            }
            await _dbContext.Spell.Set(spells.ToArray());
            spells = _mapper.Map<fb.protocol.db.Spell[], Spell[]>(request.Spells.ToArray());
            await _dbContext.Spell.Set(spells.ToArray());

            return new Response.Save
            {
                Success = true
            };
        }
    }
}