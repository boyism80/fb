using Fb.Model.EnumValue;
using Http;
using Http.Model;
using Http.Service;
using Medallion.Threading.Redis;
using Protocol = fb.protocol._internal;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Services
{
    public class FriendService
    {
        private readonly DbContext _dbContext;
        private readonly RabbitMqService _rabbitMqService;
        private readonly SessionService _sessionService;
        private readonly RedisDistributedLockService _distributedLock;

        public FriendService(DbContext dbContext,
            RabbitMqService rabbitMqService,
            SessionService sessionService,
            RedisDistributedLockService distributedLock)
        {
            _dbContext = dbContext;
            _rabbitMqService = rabbitMqService;
            _sessionService = sessionService;
            _distributedLock = distributedLock;
        }

        public async Task<List<Protocol.FriendEntry>> GetEntries(uint world, uint user)
        {
            var friends = await _dbContext.Friend.Get(world, user);
            return friends.Select(ToEntry).ToList();
        }

        public async Task<Response.UpdateFriends> Update(Request.UpdateFriends request)
        {
            var world = request.World;
            var user = request.User;
            var now = DateTime.Now;

            var friends = await FindFriends(world, user, request.Names);
            var lockUids = new SortedSet<uint> { user };
            foreach (var friend in friends)
                lockUids.Add(friend.FriendUid);

            while (true)
            {
                var locks = await AcquireFriendLocks(world, lockUids);
                try
                {
                    var existing = (await _dbContext.Friend.Get(world, user)).ToList();
                    var required = new SortedSet<uint>(lockUids);
                    foreach (var row in existing)
                        required.Add(row.FriendUid);

                    if (!required.SetEquals(lockUids))
                    {
                        lockUids = required;
                        continue;
                    }

                    var existingByFriend = existing.ToDictionary(x => x.FriendUid);
                    var newFriendUids = friends.Select(x => x.FriendUid).ToHashSet();
                    var removed = existing.Where(x => !newFriendUids.Contains(x.FriendUid)).ToList();
                    var ownerName = await _dbContext.Character.GetName(world, user) ?? string.Empty;
                    var peerUpdates = new List<PeerRelationNotify>();

                    foreach (var old in removed)
                    {
                        var reverse = await _dbContext.Friend.Get(world, old.FriendUid, user);
                        if (reverse != null && reverse.Mutual)
                        {
                            reverse.Mutual = false;
                            reverse.UpdatedDate = now;
                            _dbContext.Friend.Set(world, reverse);
                            peerUpdates.Add(new PeerRelationNotify
                            {
                                PeerUid = old.FriendUid,
                                FriendUid = user,
                                FriendName = ownerName,
                                Mutual = false
                            });
                        }
                    }

                    var nextFriends = new List<Friend>();
                    foreach (var friend in friends)
                    {
                        var reverse = await _dbContext.Friend.Get(world, friend.FriendUid, user);
                        var mutual = reverse != null;
                        var previousMutual = existingByFriend.TryGetValue(friend.FriendUid, out var prev) && prev.Mutual;

                        nextFriends.Add(new Friend
                        {
                            User = user,
                            FriendUid = friend.FriendUid,
                            FriendName = friend.FriendName,
                            Mutual = mutual,
                            CreatedDate = existingByFriend.TryGetValue(friend.FriendUid, out var existingRow)
                                ? existingRow.CreatedDate
                                : now,
                            UpdatedDate = now
                        });

                        if (reverse != null)
                        {
                            if (reverse.Mutual != mutual)
                            {
                                reverse.Mutual = mutual;
                                reverse.UpdatedDate = now;
                                _dbContext.Friend.Set(world, reverse);
                            }

                            if (previousMutual != mutual)
                            {
                                peerUpdates.Add(new PeerRelationNotify
                                {
                                    PeerUid = friend.FriendUid,
                                    FriendUid = user,
                                    FriendName = ownerName,
                                    Mutual = mutual
                                });
                            }
                        }
                    }

                    ApplyHashEntitySnapshot(
                        nextFriends.ToArray(),
                        existing,
                        removedList => _dbContext.Friend.Delete(world, removedList),
                        alive => _dbContext.Friend.Set(world, alive));

                    await _dbContext.SaveChangesAsync();
                    await NotifyPeerRelations(world, peerUpdates);

                    return new Response.UpdateFriends
                    {
                        Error = (uint)ErrorCode.None,
                        Friends = nextFriends.Select(ToEntry).ToList()
                    };
                }
                finally
                {
                    await ReleaseLocks(locks);
                }
            }
        }

        public async Task<Response.FriendBroadcast> Broadcast(Request.FriendBroadcast request)
        {
            var world = request.World;
            var allowed = (await _dbContext.Friend.Get(world, request.FromUid))
                .Where(f => f.Mutual)
                .Select(f => f.FriendUid)
                .ToHashSet();

            var targets = (request.ToUids ?? new List<uint>())
                .Where(uid => allowed.Contains(uid))
                .Distinct()
                .ToList();

            foreach (var toUid in targets)
            {
                var toName = await _dbContext.Character.GetName(world, toUid);
                if (string.IsNullOrEmpty(toName))
                    continue;

                var session = await _sessionService.Get(world, toName);
                if (session == null)
                    continue;
                if (session.Host == request.Host && AmqpRoute.Parse(session.Role) == request.Role)
                    continue;

                await _rabbitMqService.PublishAsync(new Response.FriendMessage
                {
                    ToUid = toUid,
                    Message = request.Message,
                    Type = request.Type
                }, AmqpRoute.Exchange, AmqpRoute.Unicast(session, world));
            }

            return new Response.FriendBroadcast
            {
                Error = (uint)ErrorCode.None
            };
        }

        private async Task<List<NamedFriend>> FindFriends(uint world, uint user, List<string> names)
        {
            var friends = new List<NamedFriend>();
            var seen = new HashSet<uint>();
            foreach (var name in names ?? new List<string>())
            {
                if (string.IsNullOrWhiteSpace(name))
                    continue;

                var friendId = await _dbContext.Character.GetCharacterId(world, name);
                if (!friendId.HasValue || friendId.Value == user)
                    continue;
                if (!seen.Add(friendId.Value))
                    continue;

                var canonicalName = await _dbContext.Character.GetName(world, friendId.Value) ?? name;
                friends.Add(new NamedFriend
                {
                    FriendUid = friendId.Value,
                    FriendName = canonicalName
                });
            }

            return friends;
        }

        private async Task NotifyPeerRelations(uint world, List<PeerRelationNotify> peerUpdates)
        {
            var distinct = peerUpdates
                .GroupBy(x => new { x.PeerUid, x.FriendUid, x.FriendName, x.Mutual })
                .Select(g => g.First());

            foreach (var update in distinct)
            {
                var peerName = await _dbContext.Character.GetName(world, update.PeerUid);
                if (string.IsNullOrEmpty(peerName))
                    continue;

                var peerSession = await _sessionService.Get(world, peerName);
                if (peerSession == null)
                    continue;

                await _rabbitMqService.PublishAsync(new Response.FriendRelation
                {
                    User = update.PeerUid,
                    FriendUid = update.FriendUid,
                    FriendName = update.FriendName,
                    Mutual = update.Mutual
                }, AmqpRoute.Exchange, AmqpRoute.Unicast(peerSession, world));
            }
        }

        private async Task<List<RedisDistributedLockHandle>> AcquireFriendLocks(uint world, SortedSet<uint> uids)
        {
            var locks = new List<RedisDistributedLockHandle>();
            foreach (var uid in uids)
                locks.Add(await _distributedLock.Lock(world, Friend.DistributedLockKey(uid)));
            return locks;
        }

        private static async Task ReleaseLocks(List<RedisDistributedLockHandle> locks)
        {
            for (var i = locks.Count - 1; i >= 0; i--)
                await locks[i].DisposeAsync();
        }

        private static void ApplyHashEntitySnapshot(
            Friend[] request,
            IReadOnlyList<Friend> existing,
            Action<IReadOnlyList<Friend>> deleteMany,
            Action<Friend[]> setMany)
        {
            var src = request.ToDictionary(x => $"{x.GetRedisKey()}:{x.GetRedisField()}");
            var dst = existing.ToDictionary(x => $"{x.GetRedisKey()}:{x.GetRedisField()}");

            var removed = dst.Keys.Except(src.Keys).Select(k => dst[k]).ToList();
            if (removed.Count > 0)
                deleteMany(removed);

            if (request.Length > 0)
                setMany(request);
        }

        private static Protocol.FriendEntry ToEntry(Friend friend)
        {
            return new Protocol.FriendEntry
            {
                Name = friend.FriendName,
                Uid = friend.FriendUid,
                Mutual = friend.Mutual
            };
        }

        private sealed class NamedFriend
        {
            public uint FriendUid { get; init; }
            public string FriendName { get; init; } = string.Empty;
        }

        private sealed class PeerRelationNotify
        {
            public uint PeerUid { get; init; }
            public uint FriendUid { get; init; }
            public string FriendName { get; init; } = string.Empty;
            public bool Mutual { get; init; }
        }
    }
}
