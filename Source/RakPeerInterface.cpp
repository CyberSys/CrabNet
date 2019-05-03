#include "RakPeerInterface.h"
using namespace CrabNet;

#if defined(_WIN32)

#include "GetTime.h"
#include "RakSleep.h"

uint64_t RakPeerInterface::Get64BitUniqueRandomNumber()
{
    uint64_t g=CrabNet::GetTimeUS();

    CrabNet::TimeUS lastTime, thisTime;
    int j;
    // Sleep a small random time, then use the last 4 bits as a source of randomness
    for (j=0; j < 8; j++)
    {
        lastTime = CrabNet::GetTimeUS();
        RakSleep(1);
        RakSleep(0);
        thisTime = CrabNet::GetTimeUS();
        CrabNet::TimeUS diff = thisTime-lastTime;
        unsigned int diff4Bits = (unsigned int) (diff & 15);
        diff4Bits <<= 32-4;
        diff4Bits >>= j*4;
        ((char*)&g)[j] ^= diff4Bits;
    }
    return g;
}
#else
#include "gettimeofday.h"

uint64_t RakPeerInterface::Get64BitUniqueRandomNumber()
{
    // Mac address is a poor solution because you can't have multiple connections from the same system

    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_usec + tv.tv_sec * 1000000;
}
#endif
