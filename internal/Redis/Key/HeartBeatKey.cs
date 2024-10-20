﻿namespace Internal.Redis.Key
{
    public class HeartBeatKey : IRedisKey
    {
        public fb.protocol._internal.Service Service { get; set; }
        public byte Id { get; set; } = 0xFF;

        public string Key => $"heart-beat:{Service}:{Id}";
    }
}
