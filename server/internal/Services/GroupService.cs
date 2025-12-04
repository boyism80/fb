using Fb.Model;
using Fb.Model.EnumValue;
using Http;
using Http.Model;
using Http.Service;
using Protocol = fb.protocol._internal;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Services
{
    /// <summary>
    /// Provides group management service functionality.
    /// Handles group creation, member management, and group operations.
    /// </summary>
    public class GroupService
    {
        private readonly IConfiguration _configuration;
        private readonly DbContext _dbContext;
        private readonly RabbitMqService _rabbitMqService;
        private readonly SessionService _sessionService;
        private readonly RedisService _redisService;
        private readonly RedisDistributedLockService _distributedLock;

        /// <summary>
        /// Initializes a new instance of the <see cref="GroupService"/> class.
        /// </summary>
        public GroupService(IConfiguration configuration,
            DbContext dbContext,
            RabbitMqService rabbitMqService,
            SessionService sessionService,
            RedisService redisService,
            RedisDistributedLockService distributedLock)
        {
            _configuration = configuration;
            _dbContext = dbContext;
            _rabbitMqService = rabbitMqService;
            _sessionService = sessionService;
            _redisService = redisService;
            _distributedLock = distributedLock;
        }

        /// <summary>
        /// Gets group details by group ID.
        /// </summary>
        public async Task<Response.GroupDetails> Get(uint id)
        {
            try
            {
                var redis = _redisService.Redis(id).Connection;

                await using (await _distributedLock.Lock(Group.DistributedLockKey(id)))
                {
                    var group = await _dbContext.Group.Get(id) ??
                        throw new LogicException(ErrorCode.GroupNotFound);

                    var master = await _dbContext.Character.Get(group.Master) ??
                        throw new LogicException(ErrorCode.NotFoundCharacter);

                    var members = new List<Protocol.CharacterRef>();
                    foreach (var uid in group.Members)
                    {
                        var ch = await _dbContext.Character.Get(uid) ??
                            throw new LogicException(ErrorCode.NotFoundCharacter);
                        members.Add(new Protocol.CharacterRef
                        {
                            Uid = ch.Id,
                            Name = ch.Name
                        });
                    }

                    // Add master to members list
                    members.Add(new Protocol.CharacterRef
                    {
                        Uid = master.Id,
                        Name = master.Name
                    });

                    return new Response.GroupDetails
                    {
                        Host = _configuration.GetValue<uint>("id"),
                        Action = Protocol.GroupDetailsAction.Query,
                        Group = new Protocol.Group
                        {
                            Id = group.Master,
                            Master = master.Name,
                            Members = new List<string>() // Empty for GroupDetails, members are in CharacterRef array
                        },
                        Members = members,
                        Error = (uint)ErrorCode.None
                    };
                }
            }
            catch (LogicException e)
            {
                return new Response.GroupDetails
                {
                    Host = _configuration.GetValue<uint>("id"),
                    Action = Protocol.GroupDetailsAction.Query,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.GroupDetails
                {
                    Host = _configuration.GetValue<uint>("id"),
                    Action = Protocol.GroupDetailsAction.Query,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        /// <summary>
        /// Creates a new group with the specified master and member.
        /// </summary>
        public async Task<Response.GroupDetails> Create(Request.CreateGroup request)
        {
            try
            {
                var actor = await _dbContext.Character.Get(request.Master) ??
                    throw new Exception($"user {request.Master} not found");

                if (actor.Name == request.Member)
                    throw new LogicException(ErrorCode.CannotGroupSelf);

                if (await _sessionService.Get(actor.Name) == null)
                    throw new Exception($"user {request.Master} is offline");

                if (Table.Map.TryGetValue(actor.Map, out var map) == false)
                    throw new Exception("invalid map");

                var targetSession = await _sessionService.Get(request.Member) ??
                    throw new LogicException(ErrorCode.Offline);

                var target = await _dbContext.Character.Get(targetSession.Uid) ??
                    throw new LogicException(ErrorCode.Offline);

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(actor.Id)))
                {
                    await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(target.Id)))
                    {
                        var actorSync = await _dbContext.CharacterSync.Get(actor.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        var targetSync = await _dbContext.CharacterSync.Get(target.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        // Create group - actor must not be in a group
                        if (actorSync.Group != null)
                            throw new LogicException(ErrorCode.GroupAlreadyJoined);

                        if (targetSync.Group != null)
                            throw new LogicException(ErrorCode.GroupTargetAlreadyJoined);

                        var masterSetting = await _dbContext.Option.Get(actor.Id) ??
                            throw new Exception($"user option {request.Master} not found");

                        if (masterSetting.Group == false)
                            throw new LogicException(ErrorCode.DisabledGroup);

                        var memberSetting = await _dbContext.Option.Get(target.Id) ??
                            throw new Exception($"user option {request.Member} not found");

                        if (memberSetting.Group == false)
                            throw new LogicException(ErrorCode.DisabledGroupTarget);

                        await using (await _distributedLock.Lock(Group.DistributedLockKey(actor.Id)))
                        {
                            var group = new Group
                            {
                                Master = actor.Id,
                            };
                            group.Members.Add(target.Id);

                            actorSync.Group = group.Master;
                            targetSync.Group = group.Master;
                            _dbContext.CharacterSync.Set(actorSync);
                            _dbContext.CharacterSync.Set(targetSync);
                            _dbContext.Group.Set(group);

                            var members = new List<Protocol.CharacterRef>();
                            members.Add(new Protocol.CharacterRef
                            {
                                Uid = actor.Id,
                                Name = actor.Name
                            });
                            members.Add(new Protocol.CharacterRef
                            {
                                Uid = target.Id,
                                Name = target.Name
                            });

                            await _dbContext.SaveChangesAsync();

                            var response = new Response.GroupDetails
                            {
                                Host = map.Host,
                                Action = Protocol.GroupDetailsAction.Create,
                                Group = new Protocol.Group
                                {
                                    Id = group.Master,
                                    Master = actor.Name,
                                    Members = new List<string>()
                                },
                                Members = members,
                                Error = (uint)ErrorCode.None
                            };

                            _rabbitMqService.Publish(response, "amq.direct", $"fb.group");
                            return response;
                        }
                    }
                }
            }
            catch (LogicException e)
            {
                return new Response.GroupDetails
                {
                    Host = _configuration.GetValue<uint>("id"),
                    Action = Protocol.GroupDetailsAction.Create,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.GroupDetails
                {
                    Host = _configuration.GetValue<uint>("id"),
                    Action = Protocol.GroupDetailsAction.Create,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        /// <summary>
        /// Adds a member to an existing group.
        /// </summary>
        public async Task<Response.UpdatedGroup> Enter(Request.EnterGroup request)
        {
            try
            {
                var actor = await _dbContext.Character.Get(request.Master) ??
                    throw new Exception($"user {request.Master} not found");

                if (actor.Name == request.Member)
                    throw new LogicException(ErrorCode.CannotGroupSelf);

                if (await _sessionService.Get(actor.Name) == null)
                    throw new Exception($"user {request.Master} is offline");

                if (Table.Map.TryGetValue(actor.Map, out var map) == false)
                    throw new Exception("invalid map");

                var targetSession = await _sessionService.Get(request.Member) ??
                    throw new LogicException(ErrorCode.Offline);

                var target = await _dbContext.Character.Get(targetSession.Uid) ??
                    throw new LogicException(ErrorCode.Offline);

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(actor.Id)))
                {
                    await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(target.Id)))
                    {
                        var actorSync = await _dbContext.CharacterSync.Get(actor.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        var targetSync = await _dbContext.CharacterSync.Get(target.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        // Enter group - actor must be group master
                        if (actorSync.Group == null)
                            throw new LogicException(ErrorCode.GroupNotJoined);

                        if (actorSync.Group.Value != actor.Id)
                            throw new LogicException(ErrorCode.NotGroupMaster);

                        if (targetSync.Group != null)
                            throw new LogicException(ErrorCode.GroupTargetAlreadyJoined);

                        var masterSetting = await _dbContext.Option.Get(actor.Id) ??
                            throw new Exception($"user option {request.Master} not found");

                        if (masterSetting.Group == false)
                            throw new LogicException(ErrorCode.DisabledGroup);

                        var memberSetting = await _dbContext.Option.Get(target.Id) ??
                            throw new Exception($"user option {request.Member} not found");

                        if (memberSetting.Group == false)
                            throw new LogicException(ErrorCode.DisabledGroupTarget);

                        await using (await _distributedLock.Lock(Group.DistributedLockKey(actor.Id)))
                        {
                            return await EnterInternal(actor, target, actorSync, targetSync, map);
                        }
                    }
                }
            }
            catch (LogicException e)
            {
                return new Response.UpdatedGroup
                {
                    Host = _configuration.GetValue<uint>("id"),
                    Action = Protocol.GroupActionType.Enter,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.UpdatedGroup
                {
                    Host = _configuration.GetValue<uint>("id"),
                    Action = Protocol.GroupActionType.Enter,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        /// <summary>
        /// Removes a member from a group.
        /// </summary>
        public async Task<Response.UpdatedGroup> Leave(Request.LeaveGroup request)
        {
            try
            {
                var session = await _sessionService.Get(request.Member) ??
                    throw new LogicException(ErrorCode.Offline);

                var character = await _dbContext.Character.Get(session.Uid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                if (Table.Map.TryGetValue(character.Map, out var map) == false)
                    throw new LogicException(ErrorCode.NotFoundMap);

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(character.Id)))
                {
                    var sync = await _dbContext.CharacterSync.Get(character.Id) ??
                        throw new LogicException(ErrorCode.NotFoundCharacterSync);

                    var groupId = sync.Group ??
                        throw new LogicException(ErrorCode.GroupNotJoined);

                    await using (await _distributedLock.Lock(Group.DistributedLockKey(groupId)))
                    {
                        var group = await _dbContext.Group.Get(groupId) ??
                            throw new LogicException(ErrorCode.GroupNotFound);

                        // Leave group - character must not be master
                        if (character.Id == group.Master)
                            throw new LogicException(ErrorCode.NotGroupMaster); // Use destroy endpoint instead

                        if (!group.Members.Contains(character.Id))
                            throw new LogicException(ErrorCode.GroupNotJoined);

                        group.Members.Remove(character.Id);
                        _dbContext.Group.Set(group);

                        sync.Group = null;
                        _dbContext.CharacterSync.Set(sync);

                        var master = await _dbContext.Character.Get(group.Master) ??
                            throw new LogicException(ErrorCode.NotFoundCharacter);

                        await _dbContext.SaveChangesAsync();

                        var response = new Response.UpdatedGroup
                        {
                            Host = map.Host,
                            Action = Protocol.GroupActionType.Leave,
                            GroupId = group.Master,
                            GroupMaster = master.Name,
                            Actor = new Protocol.CharacterRef
                            {
                                Uid = character.Id,
                                Name = character.Name
                            },
                            DeletedMember = new Protocol.CharacterRef
                            {
                                Uid = character.Id,
                                Name = character.Name
                            },
                            Error = (uint)ErrorCode.None
                        };

                        _rabbitMqService.Publish(response, "amq.direct", $"fb.group");
                        return response;
                    }
                }
            }
            catch (LogicException e)
            {
                return new Response.UpdatedGroup
                {
                    Host = _configuration.GetValue<uint>("id"),
                    Action = Protocol.GroupActionType.Leave,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.UpdatedGroup
                {
                    Host = _configuration.GetValue<uint>("id"),
                    Action = Protocol.GroupActionType.Leave,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        /// <summary>
        /// Kicks a member from a group by the group master.
        /// </summary>
        public async Task<Response.UpdatedGroup> Kick(Request.KickGroup request)
        {
            try
            {
                // Get actor session and character
                var actorSession = await _sessionService.Get(request.Kicker) ??
                    throw new LogicException(ErrorCode.Offline);

                var actor = await _dbContext.Character.Get(actorSession.Uid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                // Get target character
                var targetUid = await _dbContext.Character.GetCharacterId(request.Target) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);
                var target = await _dbContext.Character.Get(targetUid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                // Prevent self-kicking
                if (actor.Name == target.Name)
                    throw new LogicException(ErrorCode.CannotGroupSelf);

                if (Table.Map.TryGetValue(actor.Map, out var map) == false)
                    throw new LogicException(ErrorCode.NotFoundMap);

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(actor.Id)))
                {
                    await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(target.Id)))
                    {
                        var actorSync = await _dbContext.CharacterSync.Get(actor.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        var targetSync = await _dbContext.CharacterSync.Get(target.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        // Check if actor is in a group
                        if (actorSync.Group == null)
                            throw new LogicException(ErrorCode.GroupNotJoined);

                        // Check if target is in the same group
                        if (targetSync.Group != actorSync.Group)
                            throw new LogicException(ErrorCode.GroupNotJoined);

                        await using (await _distributedLock.Lock(Group.DistributedLockKey(actorSync.Group.Value)))
                        {
                            return await KickInternal(actor, target, actorSync, targetSync, map);
                        }
                    }
                }
            }
            catch (LogicException e)
            {
                return new Response.UpdatedGroup
                {
                    Host = _configuration.GetValue<uint>("id"),
                    Action = Protocol.GroupActionType.Kick,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.UpdatedGroup
                {
                    Host = _configuration.GetValue<uint>("id"),
                    Action = Protocol.GroupActionType.Kick,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        /// <summary>
        /// Destroys a group by the group master.
        /// </summary>
        public async Task<Response.DestroyGroup> Destroy(Request.DestroyGroup request)
        {
            try
            {
                var session = await _sessionService.Get(request.Master) ??
                    throw new LogicException(ErrorCode.Offline);

                var character = await _dbContext.Character.Get(session.Uid) ??
                    throw new LogicException(ErrorCode.NotFoundCharacter);

                if (Table.Map.TryGetValue(character.Map, out var map) == false)
                    throw new LogicException(ErrorCode.NotFoundMap);

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(character.Id)))
                {
                    var sync = await _dbContext.CharacterSync.Get(character.Id) ??
                        throw new LogicException(ErrorCode.NotFoundCharacterSync);

                    var groupId = sync.Group ??
                        throw new LogicException(ErrorCode.GroupNotJoined);

                    await using (await _distributedLock.Lock(Group.DistributedLockKey(groupId)))
                    {
                        var group = await _dbContext.Group.Get(groupId) ??
                            throw new LogicException(ErrorCode.GroupNotFound);

                        // Destroy group - character must be master
                        if (group.Master != character.Id)
                            throw new LogicException(ErrorCode.NotGroupMaster);

                        // Remove all members from group
                        var memberNames = new List<string>();
                        foreach (var uid in group.Members)
                        {
                            await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(uid)))
                            {
                                var member = await _dbContext.Character.Get(uid) ??
                                    throw new LogicException(ErrorCode.NotFoundCharacter);

                                var memberSync = await _dbContext.CharacterSync.Get(uid) ??
                                    throw new LogicException(ErrorCode.NotFoundCharacterSync);

                                memberSync.Group = null;
                                _dbContext.CharacterSync.Set(memberSync);
                                memberNames.Add(member.Name);
                            }
                        }

                        sync.Group = null;
                        _dbContext.CharacterSync.Set(sync);

                        group.Deleted = true;
                        _dbContext.Group.Set(group);

                        await _dbContext.SaveChangesAsync();

                        var response = new Response.DestroyGroup
                        {
                            Host = map.Host,
                            GroupId = group.Master,
                            GroupMaster = character.Name,
                            Actor = new Protocol.CharacterRef
                            {
                                Uid = character.Id,
                                Name = character.Name
                            },
                            Error = (uint)ErrorCode.None
                        };

                        _rabbitMqService.Publish(response, "amq.direct", $"fb.group");
                        return response;
                    }
                }
            }
            catch (LogicException e)
            {
                return new Response.DestroyGroup
                {
                    Host = _configuration.GetValue<uint>("id"),
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.DestroyGroup
                {
                    Host = _configuration.GetValue<uint>("id"),
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        /// <summary>
        /// Broadcasts a message to all members of a group.
        /// </summary>
        public async Task<Response.BroadcastGroup> Broadcast(Request.BroadcastGroup request)
        {
            try
            {
                await using (await _distributedLock.Lock(Group.DistributedLockKey(request.Group)))
                {
                    var group = await _dbContext.Group.Get(request.Group) ??
                        throw new LogicException(ErrorCode.GroupNotFound);

                    var response = new Response.BroadcastGroup
                    {
                        Host = request.Host,
                        Group = request.Group,
                        Message = request.Message,
                        Type = request.Type,
                        Error = (uint)ErrorCode.None
                    };
                    _rabbitMqService.Publish(response, "amq.direct", $"fb.group");
                    return response;
                }
            }
            catch (LogicException e)
            {
                return new Response.BroadcastGroup
                {
                    Host = request.Host,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.BroadcastGroup
                {
                    Host = request.Host,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        /// <summary>
        /// Toggles a member's group status. If the member is in the same group, kicks them. Otherwise, adds them to the group.
        /// </summary>
        public async Task<Response.UpdatedGroup> Toggle(Request.EnterGroup request)
        {
            try
            {
                var actor = await _dbContext.Character.Get(request.Master) ??
                    throw new Exception($"user {request.Master} not found");

                if (actor.Name == request.Member)
                    throw new LogicException(ErrorCode.CannotGroupSelf);

                if (await _sessionService.Get(actor.Name) == null)
                    throw new Exception($"user {request.Master} is offline");

                if (Table.Map.TryGetValue(actor.Map, out var map) == false)
                    throw new Exception("invalid map");

                var targetSession = await _sessionService.Get(request.Member) ??
                    throw new LogicException(ErrorCode.Offline);

                var target = await _dbContext.Character.Get(targetSession.Uid) ??
                    throw new LogicException(ErrorCode.Offline);

                await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(actor.Id)))
                {
                    await using (await _distributedLock.Lock(CharacterSync.DistributedLockKey(target.Id)))
                    {
                        var actorSync = await _dbContext.CharacterSync.Get(actor.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        var targetSync = await _dbContext.CharacterSync.Get(target.Id) ??
                            throw new LogicException(ErrorCode.NotFoundCharacterSync);

                        // Toggle requires actor to be group master
                        if (actorSync.Group == null)
                            throw new LogicException(ErrorCode.GroupNotJoined);

                        if (actorSync.Group.Value != actor.Id)
                            throw new LogicException(ErrorCode.NotGroupMaster);

                        await using (await _distributedLock.Lock(Group.DistributedLockKey(actor.Id)))
                        {
                            // Check if target is in the same group
                            if (targetSync.Group == actorSync.Group)
                            {
                                // Kick the member
                                return await KickInternal(actor, target, actorSync, targetSync, map);
                            }
                            else
                            {
                                // Add member to group
                                if (targetSync.Group != null)
                                    throw new LogicException(ErrorCode.GroupTargetAlreadyJoined);

                                var masterSetting = await _dbContext.Option.Get(actor.Id) ??
                                    throw new Exception($"user option {request.Master} not found");

                                if (masterSetting.Group == false)
                                    throw new LogicException(ErrorCode.DisabledGroup);

                                var memberSetting = await _dbContext.Option.Get(target.Id) ??
                                    throw new Exception($"user option {request.Member} not found");

                                if (memberSetting.Group == false)
                                    throw new LogicException(ErrorCode.DisabledGroupTarget);

                                return await EnterInternal(actor, target, actorSync, targetSync, map);
                            }
                        }
                    }
                }
            }
            catch (LogicException e)
            {
                return new Response.UpdatedGroup
                {
                    Host = _configuration.GetValue<uint>("id"),
                    Action = Protocol.GroupActionType.Enter,
                    Error = (uint)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.UpdatedGroup
                {
                    Host = _configuration.GetValue<uint>("id"),
                    Action = Protocol.GroupActionType.Enter,
                    Error = (uint)ErrorCode.Unhandled
                };
            }
        }

        /// <summary>
        /// Internal method to add a member to a group. Assumes locks are already acquired.
        /// </summary>
        private async Task<Response.UpdatedGroup> EnterInternal(Character actor, Character target, CharacterSync actorSync, CharacterSync targetSync, Map map)
        {
            var group = await _dbContext.Group.Get(actor.Id) ??
                throw new LogicException(ErrorCode.GroupNotFound);

            if (group.Members.Contains(target.Id))
                throw new LogicException(ErrorCode.GroupTargetAlreadyJoined);

            group.Members.Add(target.Id);
            targetSync.Group = group.Master;
            _dbContext.Group.Set(group);
            _dbContext.CharacterSync.Set(targetSync);

            await _dbContext.SaveChangesAsync();

            var response = new Response.UpdatedGroup
            {
                Host = map.Host,
                Action = Protocol.GroupActionType.Enter,
                GroupId = group.Master,
                GroupMaster = actor.Name,
                Actor = new Protocol.CharacterRef
                {
                    Uid = actor.Id,
                    Name = actor.Name
                },
                NewMember = new Protocol.CharacterRef
                {
                    Uid = target.Id,
                    Name = target.Name
                },
                Error = (uint)ErrorCode.None
            };

            _rabbitMqService.Publish(response, "amq.direct", $"fb.group");
            return response;
        }

        /// <summary>
        /// Internal method to kick a member from a group. Assumes locks are already acquired.
        /// </summary>
        private async Task<Response.UpdatedGroup> KickInternal(Character actor, Character target, CharacterSync actorSync, CharacterSync targetSync, Map map)
        {
            var group = await _dbContext.Group.Get(actorSync.Group.Value) ??
                throw new LogicException(ErrorCode.GroupNotFound);

            // Check if actor is the group master
            if (group.Master != actor.Id)
                throw new LogicException(ErrorCode.NotGroupMaster);

            // Check if target is a member of the group
            if (!group.Members.Contains(target.Id))
                throw new LogicException(ErrorCode.GroupNotJoined);

            // Remove target from group
            group.Members.Remove(target.Id);
            _dbContext.Group.Set(group);

            targetSync.Group = null;
            _dbContext.CharacterSync.Set(targetSync);

            await _dbContext.SaveChangesAsync();

            var response = new Response.UpdatedGroup
            {
                Host = map.Host,
                Action = Protocol.GroupActionType.Kick,
                GroupId = group.Master,
                GroupMaster = actor.Name,
                Actor = new Protocol.CharacterRef
                {
                    Uid = actor.Id,
                    Name = actor.Name
                },
                DeletedMember = new Protocol.CharacterRef
                {
                    Uid = target.Id,
                    Name = target.Name
                },
                Error = (uint)ErrorCode.None
            };

            _rabbitMqService.Publish(response, "amq.direct", $"fb.group");
            return response;
        }
    }
}

