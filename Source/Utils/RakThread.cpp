/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  Copyright (c) 2016-2018, TES3MP Team
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "RakThread.h"
#include "RakAssert.h"
#include "RakNetDefines.h"
#include "RakSleep.h"

using namespace RakNet;

#include <thread>

#if defined(_WIN32)
int RakThread::Create( unsigned __stdcall start_address( void* ), void *arglist, int priority)
#else
int RakThread::Create(void* start_address( void* ), void *arglist, int)
#endif
{
    try
    {
        std::thread thr(start_address, arglist);
        thr.detach();
    }
    catch(std::exception &e)
    {
        return 1;
    }
    return 0;
}
