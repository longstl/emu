#include <protocol/readStreamsExtractor.hpp>

#include <string.h>

namespace eMU
{
namespace protocol
{

ReadStreamsExtractor::ReadStreamsExtractor(const core::network::Payload &payload):
    payload_(payload) {}

void ReadStreamsExtractor::extract()
{
    if(payload_.empty())
    {
        throw EmptyPayloadException();
    }

    size_t currentOffset = 0;

    while(currentOffset < payload_.getSize())
    {
        if(!this->validateStream(currentOffset))
        {
            throw UnknownStreamFormatException();
        }

        size_t streamOffset = this->calculateStreamOffset(currentOffset);
        size_t streamSize = this->calculateStreamSize(currentOffset);

        core::network::Payload payload;
        payload.setSize(streamSize);
        memcpy(&payload[0], &payload_[streamOffset], streamSize);

        streams_.push_back(std::move(ReadStream(payload)));

        currentOffset += streamSize + sizeof(uint32_t);
    }
}

ReadStreamsExtractor::StreamsContainer &ReadStreamsExtractor::getStreams()
{
    return streams_;
}

size_t ReadStreamsExtractor::calculateStreamSize(size_t currentOffset) const
{
    size_t size = reinterpret_cast<const uint32_t&>(payload_[currentOffset]);

    if(size == 0)
    {
        throw EmptyStreamException();
    }

    return size;
}

size_t ReadStreamsExtractor::calculateStreamOffset(size_t currentOffset) const
{
    return currentOffset + sizeof(uint32_t);
}

bool ReadStreamsExtractor::validateStream(size_t currentOffset) const
{
    size_t streamOffset = this->calculateStreamOffset(currentOffset);

    if(streamOffset >= payload_.getSize())
    {
        return false;
    }

    return (streamOffset + this->calculateStreamSize(currentOffset) <= payload_.getSize());
}

}
}
