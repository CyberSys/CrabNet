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

#include "VariableListDeltaTracker.h"

using namespace RakNet;

VariableListDeltaTracker::VariableListDeltaTracker()
{
    nextWriteIndex = 0;
}

VariableListDeltaTracker::~VariableListDeltaTracker()
{
    for (unsigned int i = 0; i < variableList.Size(); i++)
        free(variableList[i].lastData);
}

// Call before using a series of WriteVar
void VariableListDeltaTracker::StartWrite(void)
{
    nextWriteIndex = 0;
}

void VariableListDeltaTracker::FlagDirtyFromBitArray(unsigned char *bArray)
{
    for (unsigned short readOffset = 0; readOffset < variableList.Size(); readOffset++)
    {
        if ((bArray[readOffset >> 3] & (0x80 >> (readOffset & 7))) != 0)
            variableList[readOffset].isDirty = true;
    }
}

VariableListDeltaTracker::VariableLastValueNode::VariableLastValueNode()
{
    lastData = 0;
}

VariableListDeltaTracker::VariableLastValueNode::VariableLastValueNode(const unsigned char *data, size_t _byteLength)
{
    lastData = (char *) malloc(_byteLength);
    memcpy(lastData, data, _byteLength);
    byteLength = _byteLength;
    isDirty = false;
}

VariableListDeltaTracker::VariableLastValueNode::~VariableLastValueNode()
{
}
