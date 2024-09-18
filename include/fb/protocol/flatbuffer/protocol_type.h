#ifndef	__FLATBUFFER_PROTOCOL_TYPE_H__
#define	__FLATBUFFER_PROTOCOL_TYPE_H__


namespace fb { namespace protocol { namespace db { namespace request { 

enum class FlatBufferProtocolType
{
	Ping,
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
	Ping,
	Transfer,
	Whisper,
};

} } } } 

namespace fb { namespace protocol { namespace internal { namespace response { 

enum class FlatBufferProtocolType
{
	Pong,
	Transfer,
};

} } } } 

#endif