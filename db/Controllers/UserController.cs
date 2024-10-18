using AutoMapper;
using Dapper;
using db.Model;
using Db.Model;
using Db.Service;
using Microsoft.AspNetCore.Mvc;
using MySqlConnector;
using System.Data;
using System.Linq;
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

        public UserController(IConfiguration configuration, IMapper mapper, DbExecuteService dbExecuteService)
        {
            _configuration = configuration;
            _mapper = mapper;
            _dbExecuteService = dbExecuteService;
        }

        [HttpGet("uid/{name}")]
        public async Task<Response.GetUid> Uid(string name)
        {
            using var connection = new MySqlConnection(_configuration.GetConnectionString("MySql:-1"));
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

        [HttpGet("account/{uid}")]
        public async Task<Response.Account> Account(uint uid)
        {
            using var connection = new MySqlConnection(_configuration.GetConnectionString("MySql:-1"));
            var account = await connection.QueryFirstOrDefaultAsync<Account>($"SELECT id, pw, map FROM user WHERE id = {uid} LIMIT 1");
            if (account == null)
            {
                return new Response.Account
                {
                    Success = false
                };
            }
            return new Response.Account
            { 
                Pw = account.Pw,
                Map = account.Map,
                Success = true
            };
        }

        [HttpPost("reserve-name")]
        public async Task<Response.ReserveName> ReserveName(Request.ReserveName request)
        {
            // 동시성 제어 필요
            using var connection = new MySqlConnection(_configuration.GetConnectionString("MySql:-1"));
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
            using var connection = new MySqlConnection(_configuration.GetConnectionString("MySql:-1"));
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
            using var connection = new MySqlConnection(_configuration.GetConnectionString("MySql:-1"));
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

        [HttpGet("login/{uid}")]
        public async Task<Response.Login> Login(uint uid)
        {
            using var connection = new MySqlConnection(_configuration.GetConnectionString("MySql:-1"));
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
            using var connection = new MySqlConnection(_configuration.GetConnectionString("MySql:-1"));
            await connection.ExecuteAsync("USP_CHARACTER_SET", _mapper.Map<Db.Model.Character>(request.Character), commandType: CommandType.StoredProcedure);
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