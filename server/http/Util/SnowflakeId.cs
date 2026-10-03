namespace Http.Util
{
    // 63-bit id: milliseconds since Epoch (41) | host id (8) | sequence (14).
    // The game server reads the host id with the same shift (fb::game::match::ID_HOST_SHIFT).
    public sealed class SnowflakeId
    {
        public const int HostShift = 14;
        public const int TimestampShift = 22;
        public const ulong SequenceMask = (1UL << HostShift) - 1;
        public static readonly System.DateTime Epoch = new System.DateTime(2026, 1, 1, 0, 0, 0, DateTimeKind.Utc);

        private readonly object _lock = new();
        private ulong _lastTimestamp;
        private ulong _sequence;

        public ulong Next(byte hostId)
        {
            lock (_lock)
            {
                var now = (ulong)(System.DateTime.UtcNow - Epoch).TotalMilliseconds;
                if (now > _lastTimestamp)
                {
                    _lastTimestamp = now;
                    _sequence = 0;
                }
                else
                {
                    // Borrow the next millisecond instead of waiting so ids stay unique and increasing.
                    _sequence++;
                    if (_sequence > SequenceMask)
                    {
                        _lastTimestamp++;
                        _sequence = 0;
                    }
                }

                return (_lastTimestamp << TimestampShift) | ((ulong)hostId << HostShift) | _sequence;
            }
        }

        public static byte HostId(ulong id)
        {
            return (byte)((id >> HostShift) & 0xFF);
        }
    }
}
