using Dapper;
using MySql.Data.MySqlClient;
using System.Data;

namespace Db.Service
{
    public enum Sex
    { 
        MALE = 0,
        FEMALE = 1
    }

    public class User
    { 
        public uint Id { get; set; }
        public string Name { get; set; }
        public string Pw { get; set; }
        public uint? Birth { get; set; }
        public DateTime DateTime { get; set; }
        public DateTime LastLogin { get; set; }
        public bool Admin { get; set; }
        public ushort Look { get; set; }
        public ushort Color { get; set; }
        public Sex Sex { get; set; }
        public uint Nation { get; set; }
        public uint Creature { get; set; }
        public ushort Map { get; set; }
        public uint PositionX { get; set; }
        public uint PositionY { get; set; }
        public uint Direction { get; set; }
        public uint State { get; set; }
        public uint Class { get; set; }
        public uint Promotion { get; set; }
        public uint Exp { get; set; }
        public uint Money { get; set; }
        public uint DepositedMoney { get; set; }
        public ushort? Disguise { get; set; }
        public uint Hp { get; set; }
        public uint BaseHp { get; set; }
        public uint AdditionalHp { get; set; }
        public uint Mp { get; set; }
        public uint BaseMp { get; set; }
        public uint AdditionalMp { get; set; }
        public uint? WeaponColor { get; set; }
        public uint? HelmetColor { get; set; }
        public uint? ArmorColor { get; set; }
        public uint? ShieldColor { get; set; }
        public uint? RingLeftColor { get; set; }
        public uint? RingRightColor { get; set; }
        public uint? AuxTopColor { get; set; }
        public uint? AuxBotColor { get; set; }
        public uint? Clan { get; set; }
    }

    public class DbContext
    {
        public IDbConnection Connection
        {
            get
            {
                var ip = "127.0.0.1";
                var port = 3306;
                var db = "fb";
                var uid = "fb";
                var pwd = "admin";
                return new MySqlConnection($"Server={ip}; Port={port}; Database={db}; Uid={uid}; Pwd={pwd}");
            }
        }

        static DbContext()
        {
            Dapper.DefaultTypeMap.MatchNamesWithUnderscores = true;
        }

        public DbContext()
        {
            
        }

        public async Task<T> GetTAsync<T>()
        {
            using var connection = Connection;
            var obj = await connection.QueryFirstOrDefaultAsync<T>("SELECT * FROM user WHERE id = 4");
            return obj;
        }
    }
}
