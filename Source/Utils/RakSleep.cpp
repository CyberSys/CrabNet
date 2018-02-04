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


#include <thread>

#include "RakSleep.h"

void RakSleep(unsigned int ms)
{
    if(ms == 0)
        std::this_thread::yield();
    else
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
