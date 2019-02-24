//
// Created by koncord on 19.01.18.
//

#include "SplitPacketList.h"
#include <ReliabilityLayer.h>

CrabNet::SplitPacketList::SplitPacketList() : splitPacketId(0), inUse(0), reliabilityLayer(nullptr)
{

}

void CrabNet::SplitPacketList::prealloc(unsigned count, SplitPacketIdType splitPacketId)
{
    RakAssert(count > 0);
    this->splitPacketId = splitPacketId;
    packets.resize(count);
}

bool CrabNet::SplitPacketList::insert(CrabNet::InternalPacket *internalPacket)
{
    RakAssert(internalPacket->splitPacketIndex < size());
    RakAssert(splitPacketId == internalPacket->splitPacketId);

    if (packets[internalPacket->splitPacketIndex] == nullptr)
    {
        packets[internalPacket->splitPacketIndex] = internalPacket;
        ++inUse;
        return true;
    }

    // There was an attempt to rewrite packet ptr
    reliabilityLayer->FreeInternalPacketData(internalPacket);
    reliabilityLayer->ReleaseToInternalPacketPool(internalPacket);
    return false;
}

unsigned CrabNet::SplitPacketList::size() const
{
    return static_cast<unsigned>(packets.size());
}

unsigned CrabNet::SplitPacketList::count() const
{
    return inUse;
}

CrabNet::InternalPacket *CrabNet::SplitPacketList::operator[](unsigned n)
{
    RakAssert(n < size());
    return packets[n];
}

CrabNet::SplitPacketIdType CrabNet::SplitPacketList::id() const
{
    return splitPacketId;
}
