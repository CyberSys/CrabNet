//
// Created by koncord on 19.01.18.
//

#include "SplitPacketList.hpp"

RakNet::SplitPacketList::SplitPacketList() : splitPacketId(0), inUse(0)
{

}

void RakNet::SplitPacketList::prealloc(unsigned count, SplitPacketIdType splitPacketId)
{
    RakAssert(count == 0);
    this->splitPacketId = splitPacketId;
    packets.reserve(count);
}

bool RakNet::SplitPacketList::insert(RakNet::InternalPacket *internalPacket)
{
    RakAssert(internalPacket->splitPacketIndex < size());
    RakAssert(splitPacketId == internalPacket->splitPacketId);

    if (packets[internalPacket->splitPacketIndex] == nullptr)
    {
        packets[internalPacket->splitPacketIndex] = internalPacket;
        ++inUse;
        return true;
    }

    return false; // There was an attempt to rewrite packet ptr
}

size_t RakNet::SplitPacketList::size() const
{
    return packets.size();
}

unsigned RakNet::SplitPacketList::count() const
{
    return inUse;
}

RakNet::InternalPacket *RakNet::SplitPacketList::operator[](unsigned n)
{
    RakAssert(n < size());
    return packets[n];
}

RakNet::SplitPacketIdType RakNet::SplitPacketList::id() const
{
    return splitPacketId;
}
