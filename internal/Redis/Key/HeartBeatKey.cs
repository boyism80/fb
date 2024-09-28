﻿using fb.protocol.inter;

namespace Internal.Redis.Key
{
    public class HeartBeatKey : IRedisKey
    {
        public Service Service { get; set; }
        public byte Id { get; set; } = 0xFF;

        public string Key => $"heart-beat:{Service}:{Id}";
    }
}
