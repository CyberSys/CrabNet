//
// Created by koncord on 19.01.18.
//

#pragma once

#include <InternalPacket.h>
#include <vector>

namespace RakNet
{
    class SplitPacketList
    {
        friend class ReliabilityLayer;
    public:
        SplitPacketList();
        ~SplitPacketList() = default;
        void prealloc(unsigned count, SplitPacketIdType splitPacketId);
        bool insert(InternalPacket *internalPacket);
        size_t size() const;
        unsigned count() const;

        InternalPacket *operator[](unsigned n);
        SplitPacketIdType id() const;
    private:
        std::vector<InternalPacket*> packets;
        SplitPacketIdType splitPacketId;
        SplitPacketIndexType inUse;
    };
}


