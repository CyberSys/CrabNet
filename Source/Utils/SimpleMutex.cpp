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

/// \file
///

#include "SimpleMutex.h"
#include "RakAssert.h"

using namespace RakNet;

SimpleMutex::SimpleMutex()
{
}

SimpleMutex::~SimpleMutex()
{
}

void SimpleMutex::lock()
{
    mutex.lock();
}

void SimpleMutex::unlock()
{
    mutex.unlock();
}
