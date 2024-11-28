using AutoMapper;
using Dapper;
using Fb.Model.EnumValue;
using Http;
using Http.Model;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using System.Data;
using System.Security.Cryptography;
using System.Text;
using Protocol = fb.protocol._internal;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
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
            var ch = await _dbContext.Character.Get(request.Uid);
            if (ch == null)
            {
                return new Response.Authenticate
                {
                    ErrorCode = 1
                };
            }

            if (ch.Pw != SHA256Hash(request.Pw))
            {
                return new Response.Authenticate
                {
                    ErrorCode = 2
                };
            }

            return new Response.Authenticate
            {
                ErrorCode = 0,
                Map = ch.Map
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
            var ch = new Character
            {
                Id = request.Uid,
                Name = request.Name,
                Pw = SHA256Hash(request.Pw),
                Hp = request.Hp,
                BaseHp = request.Hp,
                Mp = request.Mp,
                BaseMp = request.Mp,
                Map = request.Map,
                PositionX = request.X,
                PositionY = request.Y,
                Admin = request.Admin
            };
            _dbContext.Character.Set(ch);

            await _dbContext.SaveChangesAsync();
            return new Response.InitCharacter
            {
                Success = true
            };
        }

        [HttpPost("mk-ch")]
        public async Task<Response.MakeCharacter> MakeCharacter(Request.MakeCharacter request)
        {
            try
            {
                var ch = await _dbContext.Character.Get(request.Uid) ??
                    throw new Exception($"user {request.Uid} not found");

                ch.Look = request.Hair;
                ch.Sex = request.Sex;
                ch.Nation = request.Nation;
                ch.Creature = request.Creature;
                _dbContext.Character.Set(ch);

                var option = new Option
                {
                    Uid = request.Uid,
                };
                _dbContext.Option.Set(option);

                await _dbContext.SaveChangesAsync();
                return new Response.MakeCharacter
                {
                    Success = true
                };
            }
            catch (Exception)
            {
                return new Response.MakeCharacter
                {
                    Success = false
                };
            }
        }

        [HttpPost("change-pw")]
        public async Task<Response.ChangePw> ChangePassword(Request.ChangePw request)
        {
            try
            {
                var ch = await _dbContext.Character.Get(request.Uid) ??
                    throw new Exception($"user {request.Uid} not found");

                if (ch.Pw != SHA256Hash(request.Before))
                    throw new LogicException(ErrorCode.PasswordNotMatched);

                if (ch.Birth != request.Birthday)
                    throw new LogicException(ErrorCode.BirthdayNotMatched);

                ch.Pw = SHA256Hash(request.After);
                _dbContext.Character.Set(ch);

                await _dbContext.SaveChangesAsync();
                return new Response.ChangePw
                {
                    ErrorCode = 0
                };
            }
            catch (LogicException e)
            {
                return new Response.ChangePw
                {
                    ErrorCode = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.ChangePw
                {
                    ErrorCode = uint.MaxValue
                };
            }
        }

        [HttpGet("init/{uid}")]
        public async Task<Response.Init> Init(uint uid)
        {
            var ch = await _dbContext.Character.Get(uid);
            var items = await _dbContext.Item.Get(uid);
            var spells = await _dbContext.Spell.Get(uid);
            var option = await _dbContext.Option.Get(uid);

            var response = new Response.Init
            {
                Character = _mapper.Map<Protocol.Character>(ch),
                Items = items.Where(x => !x.Deleted).Select(_mapper.Map<Protocol.Item>).ToList(),
                Spells = spells.Where(x => !x.Deleted).Select(_mapper.Map<Protocol.Spell>).ToList(),
                Option = _mapper.Map<Protocol.Option>(option)
            };

            return response;
        }

        private static T[] Override<T>(IEnumerable<T> request, IEnumerable<T> exists) where T : IModel, IRedisHashKey
        {
            var src = request.ToDictionary(x => $"{x.GetRedisKey()}:{x.GetRedisField()}");
            var dst = exists.ToDictionary(x => $"{x.GetRedisKey()}:{x.GetRedisField()}");

            foreach (var x in dst.Values)
            {
                x.Deleted = true;
            }

            foreach (var key in src.Keys.ToArray())
            {
                dst[key] = src[key];
            }

            return dst.Values.ToArray();
        }

        [HttpPost("save")]
        public async Task<Response.Save> Save(Request.Save request)
        {
            try
            {
                var exists = await _dbContext.Character.Get(request.Character.Id) ??
                    throw new Exception();

                if (exists.Deleted)
                    throw new Exception();

                var ch = _mapper.Map<Character>(request.Character);
                _dbContext.Character.Set(ch);

                var items = Override(_mapper.Map<Protocol.Item[], Item[]>(request.Items.ToArray()), await _dbContext.Item.Get(request.Character.Id));
                _dbContext.Item.Set(items);

                var spells = Override(_mapper.Map<Protocol.Spell[], Spell[]>(request.Spells.ToArray()), await _dbContext.Spell.Get(request.Character.Id));
                _dbContext.Spell.Set(spells.ToArray());

                await _dbContext.SaveChangesAsync();
                return new Response.Save
                {
                    Success = true
                };
            }
            catch (Exception)
            {
                return new Response.Save
                {
                    Success = false
                };
            }
        }

        [HttpPost("option")]
        public async Task<Response.SetOption> Option(Request.SetOption request)
        {
            try
            {
                var option = await _dbContext.Option.Get(request.User) ??
                    throw new Exception($"option {request.User} not found");

                switch ((Setting)request.Type)
                {
                    case Setting.Whisper:
                        option.Whisper = request.Enabled;
                        break;

                    case Setting.Group:
                        option.Group = request.Enabled;
                        break;

                    case Setting.Roar:
                        option.Roar = request.Enabled;
                        break;

                    case Setting.RoarWorlds:
                        option.RoarWorlds = request.Enabled;
                        break;

                    case Setting.MagicEffect:
                        option.MagicEffect = request.Enabled;
                        break;

                    case Setting.WeatherEffect:
                        option.WeatherEffect = request.Enabled;
                        break;

                    case Setting.FixedMove:
                        option.FixedMove = request.Enabled;
                        break;

                    case Setting.Trade:
                        option.Trade = request.Enabled;
                        break;

                    case Setting.FastMove:
                        option.FastMove = request.Enabled;
                        break;

                    case Setting.EffectSound:
                        option.EffectSound = request.Enabled;
                        break;

                    case Setting.PkProtect:
                        option.PkProtect = request.Enabled;
                        break;

                    default:
                        throw new Exception($"invalid option type : {request.Type}");
                }
                _dbContext.Option.Set(option);

                await _dbContext.SaveChangesAsync();
                return new Response.SetOption
                {
                    Success = true
                };
            }
            catch (Exception)
            {
                return new Response.SetOption
                {
                    Success = false
                };
            }
        }
    }
}