#ifndef	__FLATBUFFER_PROTOCOL_TYPE_H__
#define	__FLATBUFFER_PROTOCOL_TYPE_H__


namespace fb { namespace protocol { namespace db { 

enum class FlatBufferProtocolType
{
};

} } } 

namespace fb { namespace protocol { namespace db { namespace request { 

enum class FlatBufferProtocolType
{
	CreateCharacter,
	FinishCharacter,
	InitCharacter,
	Ping,
};

} } } } 

namespace fb { namespace protocol { namespace db { namespace response { 

enum class FlatBufferProtocolType
{
	CreateCharacter,
	FinishCharacter,
	InitCharacter,
};

} } } } 

namespace fb { namespace protocol { namespace internal { 

enum class FlatBufferProtocolType
{
};

} } } 

namespace fb { namespace protocol { namespace internal { namespace request { 

enum class FlatBufferProtocolType
{
	Login,
	Logout,
	Ping,
	Transfer,
	Whisper,
};

} } } } 

namespace fb { namespace protocol { namespace internal { namespace response { 

enum class FlatBufferProtocolType
{
	Login,
	Logout,
	Pong,
	Transfer,
};

} } } } 

namespace fb { namespace protocol { 

enum class FlatBufferProtocolType
{
};

} } 

#endif