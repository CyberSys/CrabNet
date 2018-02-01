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

#if defined(_WIN32)
#include <malloc.h>
#ifndef alloca
#define alloca _alloca
#endif
#else
#if defined (__APPLE__) || defined (__APPLE_CC__) || defined(__linux__)
#include <alloca.h>
#endif
#include <cstdlib>
#endif
