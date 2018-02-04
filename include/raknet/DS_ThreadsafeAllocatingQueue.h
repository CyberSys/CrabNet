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

/// \file DS_ThreadsafeAllocatingQueue.h
/// \internal
/// A threadsafe queue, that also uses a memory pool for allocation

#ifndef __THREADSAFE_ALLOCATING_QUEUE
#define __THREADSAFE_ALLOCATING_QUEUE

#include "DS_Queue.h"
#include "SimpleMutex.h"
#include "DS_MemoryPool.h"
#include <new>

// #if defined(new)
// #pragma push_macro("new")
// #undef new
// #define RMO_NEW_UNDEF_ALLOCATING_QUEUE
// #endif

namespace DataStructures
{

template <class structureType>
class RAK_DLL_EXPORT ThreadsafeAllocatingQueue
{
public:
    // Queue operations
    void Push(structureType *s);
    structureType *PopInaccurate();
    structureType *Pop();
    void SetPageSize(int size);
    bool IsEmpty();
    structureType * operator[] ( unsigned int position );
    void RemoveAtIndex( unsigned int position );
    unsigned int Size();

    // Memory pool operations
    structureType *Allocate();
    void Deallocate(structureType *s);
    void Clear();
protected:

    mutable MemoryPool<structureType> memoryPool;
    RakNet::SimpleMutex memoryPoolMutex;
    Queue<structureType*> queue;
    RakNet::SimpleMutex queueMutex;
};

template <class structureType>
void ThreadsafeAllocatingQueue<structureType>::Push(structureType *s)
{
    std::lock_guard<RakNet::SimpleMutex> lock(queueMutex);
    queue.Push(s);
}

template <class structureType>
structureType *ThreadsafeAllocatingQueue<structureType>::PopInaccurate()
{
    structureType *s;

    if (queue.IsEmpty())
        return 0;

    std::lock_guard<RakNet::SimpleMutex> lock(queueMutex);
    if (!queue.IsEmpty())
        s=queue.Pop();
    else
        s=0;
    return s;
}

template <class structureType>
structureType *ThreadsafeAllocatingQueue<structureType>::Pop()
{
    structureType *s;
    std::lock_guard<RakNet::SimpleMutex> lock(queueMutex);
    if (queue.IsEmpty())
        return 0;

    s = queue.Pop();
    return s;
}

template <class structureType>
structureType *ThreadsafeAllocatingQueue<structureType>::Allocate()
{
    structureType *s;
    {
        std::lock_guard<RakNet::SimpleMutex> lock(memoryPoolMutex);
        s = memoryPool.Allocate();
    }
    // Call new operator, memoryPool doesn't do this
    s = new ((void*)s) structureType;
    return s;
}
template <class structureType>
void ThreadsafeAllocatingQueue<structureType>::Deallocate(structureType *s)
{
    // Call delete operator, memory pool doesn't do this
    s->~structureType();
    std::lock_guard<RakNet::SimpleMutex> lock(memoryPoolMutex);
    memoryPool.Release(s);
}

template <class structureType>
void ThreadsafeAllocatingQueue<structureType>::Clear()
{
    std::lock_guard<RakNet::SimpleMutex> lock(memoryPoolMutex);
    for (unsigned int i = 0; i < queue.Size(); i++)
    {
        queue[i]->~structureType();
        memoryPool.Release(queue[i]);
    }
    queue.Clear();
    memoryPool.Clear();
}

template <class structureType>
void ThreadsafeAllocatingQueue<structureType>::SetPageSize(int size)
{
    memoryPool.SetPageSize(size);
}

template <class structureType>
bool ThreadsafeAllocatingQueue<structureType>::IsEmpty()
{
    bool isEmpty;
    std::lock_guard<RakNet::SimpleMutex> lock(queueMutex);
    isEmpty = queue.IsEmpty();
    return isEmpty;
}

template <class structureType>
structureType * ThreadsafeAllocatingQueue<structureType>::operator[] (unsigned int position)
{
    structureType *s;
    std::lock_guard<RakNet::SimpleMutex> lock(queueMutex);
    s = queue[position];
    return s;
}

template <class structureType>
void ThreadsafeAllocatingQueue<structureType>::RemoveAtIndex(unsigned int position)
{
    std::lock_guard<RakNet::SimpleMutex> lock(queueMutex);
    queue.RemoveAtIndex(position);
}

template <class structureType>
unsigned int ThreadsafeAllocatingQueue<structureType>::Size()
{
    unsigned int s;
    std::lock_guard<RakNet::SimpleMutex> lock(queueMutex);
    s = queue.Size();
    return s;
}

}


// #if defined(RMO_NEW_UNDEF_ALLOCATING_QUEUE)
// #pragma pop_macro("new")
// #undef RMO_NEW_UNDEF_ALLOCATING_QUEUE
// #endif

#endif
