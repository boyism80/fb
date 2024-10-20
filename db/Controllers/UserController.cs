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
        private readonly DbExecuteService _dbExecuteService;
        private readonly DbContext _dbContext;

        public UserController(IConfiguration configuration, IMapper mapper, DbExecuteService dbExecuteService, DbContext dbContext)
        {
            _configuration = configuration;
            _mapper = mapper;
            _dbExecuteService = dbExecuteService;
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
            await using var connection = _dbContext.Connection(uid);
            var character = await connection.QueryFirstOrDefaultAsync<Character>($"SELECT * FROM user WHERE id = {uid} LIMIT 1");
            var items = await connection.QueryAsync<Item>($"SELECT * FROM item WHERE owner = {uid} AND deleted = 0");
            var spells = await connection.QueryAsync<Spell>($"SELECT * FROM spell WHERE owner = {uid} AND deleted = 0");
            
            var response = new Response.Login
            {
                Character = _mapper.Map<fb.protocol.db.Character>(character),
                Items = items.Select(item => _mapper.Map<fb.protocol.db.Item>(item)).ToList(),
                Spells = spells.Select(spell => _mapper.Map<fb.protocol.db.Spell>(spell)).ToList()
            };

            return response;
        }

        [HttpPost("save")]
        public async Task<Response.Save> Save(Request.Save request)
        {
            await using var connection = _dbContext.Connection(request.Character.Id);
            var characterSql = $@"
UPDATE user
SET look = {request.Character.Look},
    color = {request.Character.Color},
    sex = {request.Character.Sex},
    nation = {request.Character.Nation},
    creature = {request.Character.Creature},
    map = {request.Character.Map},
    position_x = {request.Character.Position.X},
    position_y = {request.Character.Position.Y},
    direction = {request.Character.Direction},
    state = {request.Character.State},
    class = {request.Character.ClassType},
    promotion = {request.Character.Promotion},
    exp = {request.Character.Exp},
    money = {request.Character.Money},
    deposited_money = {request.Character.DepositedMoney},
    disguise = {request.Character.Disguise?.ToString() ?? "NULL"},
    hp = {request.Character.Hp},
    base_hp = {request.Character.BaseHp},
    additional_hp = {request.Character.AdditionalHp},
    mp = {request.Character.Mp},
    base_mp = {request.Character.BaseMp},
    additional_mp = {request.Character.AdditionalMp},
    weapon_color = {request.Character.WeaponColor?.ToString() ?? "NULL"},
    helmet_color = {request.Character.HelmetColor?.ToString() ?? "NULL"},
    armor_color = {request.Character.ArmorColor?.ToString() ?? "NULL"},
    shield_color = {request.Character.ShieldColor?.ToString() ?? "NULL"},
    ring_left_color = {request.Character.RingLeftColor?.ToString() ?? "NULL"},
    ring_right_color = {request.Character.RingRightColor?.ToString() ?? "NULL"},
    aux_top_color = {request.Character.AuxTopColor?.ToString() ?? "NULL"},
    aux_bot_color = {request.Character.AuxBotColor?.ToString() ?? "NULL"},
    clan = {request.Character.Clan?.ToString() ?? "NULL"}
WHERE user.id = {request.Character.Id} LIMIT 1;";
            await _dbExecuteService.Post(request.Character.Id, characterSql);

            if (request.Items.Count > 0)
            {
                var itemArgs = request.Items.Select(item =>
                {
                    var customName = item.CustomName;
                    if (string.IsNullOrEmpty(customName))
                        customName = "NULL";
                    else
                        customName = $"\"{customName}\"";

                    return $"({item.User}, {item.Index}, {item.Parts}, {item.Deposited}, {item.Model}, {item.Count}, {item.Durability?.ToString() ?? "NULL"}, {customName}, 0)";
                });

                var itemSql = @$"
UPDATE item SET deleted = 1 WHERE item.owner = {request.Character.Id};

INSERT INTO item (`owner`, `index`, `parts`, `deposited`, `model`, `count`, `durability`, `custom_name`, `deleted`)
VALUES {string.Join(',', itemArgs)}
ON DUPLICATE KEY UPDATE model=VALUES(model), count=VALUES(count), durability=VALUES(durability), custom_name=VALUES(custom_name), deleted=0;
";
                await _dbExecuteService.Post(request.Character.Id, itemSql);
            }

            if (request.Spells.Count > 0)
            {
                var spellArgs = request.Spells.Select(spell =>
                {
                    return $"({spell.User}, {spell.Slot}, {spell.Model}, 0)";
                });

                var spellSql = @$"
UPDATE spell SET deleted = 1 WHERE spell.owner = {request.Character.Id};

INSERT INTO spell (`owner`, `slot`, `model`, `deleted`)
VALUES {string.Join(',', spellArgs)}
ON DUPLICATE KEY UPDATE model=VALUES(model), deleted=0;
";
                await _dbExecuteService.Post(request.Character.Id, spellSql);
            }

            return new Response.Save
            { 
                Success = true
            };
        }
    }
}