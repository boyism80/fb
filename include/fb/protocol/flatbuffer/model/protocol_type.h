#ifndef	__FLATBUFFER_PROTOCOL_TYPE_H__
#define	__FLATBUFFER_PROTOCOL_TYPE_H__


namespace fb { namespace game { namespace flatbuffer { namespace db { 

enum class FlatBufferProtocolType
{
	Ping,
};

} } } } 

namespace fb { namespace game { namespace flatbuffer { namespace inter { 

enum class FlatBufferProtocolType
{
	Ping,
	Pong,
	Whisper,
};

} } } } 

#endif