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

#ifndef __THREAD_POOL_H
#define __THREAD_POOL_H

#include "DS_Queue.h"
#include "SimpleMutex.h"
#include "Export.h"
#include "RakThread.h"
#include "SignaledEvent.h"

#ifdef _MSC_VER
#pragma warning( push )
#endif

class ThreadDataInterface
{
public:
    ThreadDataInterface() {}
    virtual ~ThreadDataInterface() {}

    virtual void* PerThreadFactory(void *context)=0;
    virtual void PerThreadDestructor(void* factoryResult, void *context)=0;
};
/// A simple class to create worker threads that processes a queue of functions with data.
/// This class does not allocate or deallocate memory.  It is up to the user to handle memory management.
/// InputType and OutputType are stored directly in a queue.  For large structures, if you plan to delete from the middle of the queue,
/// you might wish to store pointers rather than the structures themselves so the array can shift efficiently.
template <class InputType, class OutputType>
struct RAK_DLL_EXPORT ThreadPool
{
    ThreadPool();
    ~ThreadPool();

    /// Start the specified number of threads.
    /// \param[in] numThreads The number of threads to start
    /// \param[in] stackSize 0 for default (except on consoles).
    /// \param[in] _perThreadInit User callback to return data stored per thread.  Pass 0 if not needed.
    /// \param[in] _perThreadDeinit User callback to destroy data stored per thread, created by _perThreadInit.  Pass 0 if not needed.
    /// \return True on success, false on failure.
    bool StartThreads(int numThreads, int stackSize, void* (*_perThreadInit)()=0, void (*_perThreadDeinit)(void*)=0);

    // Alternate form of _perThreadDataFactory, _perThreadDataDestructor
    void SetThreadDataInterface(ThreadDataInterface *tdi, void *context);

    /// Stops all threads
    void StopThreads();

    /// Adds a function to a queue with data to pass to that function.  This function will be called from the thread
    /// Memory management is your responsibility!  This class does not allocate or deallocate memory.
    /// The best way to deallocate \a inputData is in userCallback.  If you call EndThreads such that callbacks were not called, you
    /// can iterate through the inputQueue and deallocate all pending input data there
    /// The best way to deallocate output is as it is returned to you from GetOutput.  Similarly, if you end the threads such that
    /// not all output was returned, you can iterate through outputQueue and deallocate it there.
    /// \param[in] workerThreadCallback The function to call from the thread
    /// \param[in] inputData The parameter to pass to \a userCallback
    void AddInput(OutputType (*workerThreadCallback)(InputType, bool *returnOutput, void* perThreadData), InputType inputData);

    /// Adds to the output queue
    /// Use it if you want to inject output into the same queue that the system uses. Normally you would not use this. Consider it a convenience function.
    /// \param[in] outputData The output to inject
    void AddOutput(OutputType outputData);

    /// Returns true if output from GetOutput is waiting.
    /// \return true if output is waiting, false otherwise
    bool HasOutput();

    /// Inaccurate but fast version of HasOutput.  If this returns true, you should still check HasOutput for the real value.
    /// \return true if output is probably waiting, false otherwise
    bool HasOutputFast();

    /// Returns true if input from GetInput is waiting.
    /// \return true if input is waiting, false otherwise
    bool HasInput();

    /// Inaccurate but fast version of HasInput.  If this returns true, you should still check HasInput for the real value.
    /// \return true if input is probably waiting, false otherwise
    bool HasInputFast();

    /// Gets the output of a call to \a userCallback
    /// HasOutput must return true before you call this function.  Otherwise it will assert.
    /// \return The output of \a userCallback.  If you have different output signatures, it is up to you to encode the data to indicate this
    OutputType GetOutput();

    /// Clears internal buffers
    void Clear();

    /// Lock the input buffer before calling the functions InputSize, InputAtIndex, and RemoveInputAtIndex
    /// It is only necessary to lock the input or output while the threads are running
    void LockInput();

    /// Unlock the input buffer after you are done with the functions InputSize, GetInputAtIndex, and RemoveInputAtIndex
    void UnlockInput();

    /// Length of the input queue
    unsigned InputSize();

    /// Get the input at a specified index
    InputType GetInputAtIndex(unsigned index);

    /// Remove input from a specific index.  This does NOT do memory deallocation - it only removes the item from the queue
    void RemoveInputAtIndex(unsigned index);

    /// Lock the output buffer before calling the functions OutputSize, OutputAtIndex, and RemoveOutputAtIndex
    /// It is only necessary to lock the input or output while the threads are running
    void LockOutput();

    /// Unlock the output buffer after you are done with the functions OutputSize, GetOutputAtIndex, and RemoveOutputAtIndex
    void UnlockOutput();

    /// Length of the output queue
    unsigned OutputSize();

    /// Get the output at a specified index
    OutputType GetOutputAtIndex(unsigned index);

    /// Remove output from a specific index.  This does NOT do memory deallocation - it only removes the item from the queue
    void RemoveOutputAtIndex(unsigned index);

    /// Removes all items from the input queue
    void ClearInput();

    /// Removes all items from the output queue
    void ClearOutput();

    /// Are any of the threads working, or is input or output available?
    bool IsWorking();

    /// The number of currently active threads.
    int NumThreadsWorking();

    /// Did we call Start?
    bool WasStarted();

    // Block until all threads are stopped.
    bool Pause();

    // Continue running
    void Resume();

protected:
    // It is valid to cancel input before it is processed.  To do so, lock the inputQueue with inputQueueMutex,
    // Scan the list, and remove the item you don't want.
    CrabNet::SimpleMutex inputQueueMutex, outputQueueMutex, workingThreadCountMutex, runThreadsMutex;

    void* (*perThreadDataFactory)();
    void (*perThreadDataDestructor)(void*);

    // inputFunctionQueue & inputQueue are paired arrays so if you delete from one at a particular index you must delete from the other
    // at the same index
    DataStructures::Queue<OutputType (*)(InputType, bool *, void*)> inputFunctionQueue;
    DataStructures::Queue<InputType> inputQueue;
    DataStructures::Queue<OutputType> outputQueue;

    ThreadDataInterface *threadDataInterface;
    void *tdiContext;


    template <class ThreadInputType, class ThreadOutputType>
    friend RAK_THREAD_DECLARATION(WorkerThread);

    /*
#ifdef _WIN32
    friend unsigned __stdcall WorkerThread( LPVOID arguments );
#else
    friend void* WorkerThread( void* arguments );
#endif
    */

    /// \internal
    bool runThreads;
    /// \internal
    int numThreadsRunning;
    /// \internal
    int numThreadsWorking;
    /// \internal
    CrabNet::SimpleMutex numThreadsRunningMutex;

    CrabNet::SignaledEvent quitAndIncomingDataEvents;

// #if defined(SN_TARGET_PSP2)
//     CrabNet::RakThread::UltUlThreadRuntime *runtime;
// #endif
};

#include "ThreadPool.h"
#include "RakSleep.h"
#ifdef _WIN32

#else
#include <unistd.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable:4127)
#pragma warning( disable : 4701 )  // potentially uninitialized local variable 'inputData' used
#endif

template <class ThreadInputType, class ThreadOutputType>
RAK_THREAD_DECLARATION(WorkerThread)
/*
#ifdef _WIN32
unsigned __stdcall WorkerThread( LPVOID arguments )
#else
void* WorkerThread( void* arguments )
#endif
*/
{



    ThreadPool<ThreadInputType, ThreadOutputType> *threadPool = (ThreadPool<ThreadInputType, ThreadOutputType>*) arguments;


    bool returnOutput;
    ThreadOutputType (*userCallback)(ThreadInputType, bool *, void*);
    ThreadInputType inputData;
    ThreadOutputType callbackOutput;

    userCallback=0;

    void *perThreadData;
    if (threadPool->perThreadDataFactory)
        perThreadData=threadPool->perThreadDataFactory();
    else if (threadPool->threadDataInterface)
        perThreadData=threadPool->threadDataInterface->PerThreadFactory(threadPool->tdiContext);
    else
        perThreadData=0;

    // Increase numThreadsRunning
    threadPool->numThreadsRunningMutex.lock();
    ++threadPool->numThreadsRunning;
    threadPool->numThreadsRunningMutex.unlock();

    while (1)
    {
//#ifdef _WIN32
        if (userCallback==0)
        {
            threadPool->quitAndIncomingDataEvents.WaitOnEvent(1000);
        }
// #else
//         if (userCallback==0)
//             RakSleep(30);
// #endif

        threadPool->runThreadsMutex.lock();
        if (threadPool->runThreads==false)
        {
            threadPool->runThreadsMutex.unlock();
            break;
        }
        threadPool->runThreadsMutex.unlock();

        threadPool->workingThreadCountMutex.lock();
        ++threadPool->numThreadsWorking;
        threadPool->workingThreadCountMutex.unlock();

        // Read input data
        userCallback=0;
        threadPool->inputQueueMutex.lock();
        if (threadPool->inputFunctionQueue.Size())
        {
            userCallback=threadPool->inputFunctionQueue.Pop();
            inputData=threadPool->inputQueue.Pop();
        }
        threadPool->inputQueueMutex.unlock();

        if (userCallback)
        {
            callbackOutput=userCallback(inputData, &returnOutput,perThreadData);
            if (returnOutput)
            {
                threadPool->outputQueueMutex.lock();
                threadPool->outputQueue.Push(callbackOutput);
                threadPool->outputQueueMutex.unlock();
            }
        }

        threadPool->workingThreadCountMutex.lock();
        --threadPool->numThreadsWorking;
        threadPool->workingThreadCountMutex.unlock();
    }

    // Decrease numThreadsRunning
    threadPool->numThreadsRunningMutex.lock();
    --threadPool->numThreadsRunning;
    threadPool->numThreadsRunningMutex.unlock();

    if (threadPool->perThreadDataDestructor)
        threadPool->perThreadDataDestructor(perThreadData);
    else if (threadPool->threadDataInterface)
        threadPool->threadDataInterface->PerThreadDestructor(perThreadData, threadPool->tdiContext);

    return 0;
}

template <class InputType, class OutputType>
ThreadPool<InputType, OutputType>::ThreadPool()
{
    runThreads=false;
    numThreadsRunning=0;
    threadDataInterface=0;
    tdiContext=0;
    numThreadsWorking=0;

}
template <class InputType, class OutputType>
ThreadPool<InputType, OutputType>::~ThreadPool()
{
    StopThreads();
    Clear();
}
template <class InputType, class OutputType>
bool ThreadPool<InputType, OutputType>::StartThreads(int numThreads, int stackSize, void* (*_perThreadDataFactory)(), void (*_perThreadDataDestructor)(void *))
{
    (void) stackSize;

// #if defined(SN_TARGET_PSP2)
//     runtime = CrabNet::RakThread::AllocRuntime(numThreads);
// #endif

    runThreadsMutex.lock();
    if (runThreads==true)
    {
        // Already running
        runThreadsMutex.unlock();
        return false;
    }
    runThreadsMutex.unlock();

    perThreadDataFactory=_perThreadDataFactory;
    perThreadDataDestructor=_perThreadDataDestructor;

    runThreadsMutex.lock();
    runThreads=true;
    runThreadsMutex.unlock();

    numThreadsWorking=0;
    unsigned threadId = 0;
    (void) threadId;
    int i;
    for (i=0; i < numThreads; i++)
    {
        int errorCode;




        errorCode = CrabNet::RakThread::Create(WorkerThread<InputType, OutputType>, this);

        if (errorCode!=0)
        {
            StopThreads();
            return false;
        }
    }
    // Wait for number of threads running to increase to numThreads
    bool done=false;
    while (done==false)
    {
        RakSleep(50);
        numThreadsRunningMutex.lock();
        if (numThreadsRunning==numThreads)
            done=true;
        numThreadsRunningMutex.unlock();
    }

    return true;
}
template <class InputType, class OutputType>
void ThreadPool<InputType, OutputType>::SetThreadDataInterface(ThreadDataInterface *tdi, void *context)
{
    threadDataInterface=tdi;
    tdiContext=context;
}
template <class InputType, class OutputType>
void ThreadPool<InputType, OutputType>::StopThreads()
{
    runThreadsMutex.lock();
    if (runThreads==false)
    {
        runThreadsMutex.unlock();
        return;
    }

    runThreads=false;
    runThreadsMutex.unlock();

    // Wait for number of threads running to decrease to 0
    bool done=false;
    while (done==false)
    {
        quitAndIncomingDataEvents.SetEvent();

        RakSleep(50);
        numThreadsRunningMutex.lock();
        if (numThreadsRunning==0)
            done=true;
        numThreadsRunningMutex.unlock();
    }

    quitAndIncomingDataEvents.CloseEvent();

// #if defined(SN_TARGET_PSP2)
//     CrabNet::RakThread::DeallocRuntime(runtime);
//     runtime=0;
// #endif

}
template <class InputType, class OutputType>
void ThreadPool<InputType, OutputType>::AddInput(OutputType (*workerThreadCallback)(InputType, bool *returnOutput, void* perThreadData), InputType inputData)
{
    inputQueueMutex.lock();
    inputQueue.Push(inputData);
    inputFunctionQueue.Push(workerThreadCallback);
    inputQueueMutex.unlock();

    quitAndIncomingDataEvents.SetEvent();
}
template <class InputType, class OutputType>
void ThreadPool<InputType, OutputType>::AddOutput(OutputType outputData)
{
    outputQueueMutex.lock();
    outputQueue.Push(outputData);
    outputQueueMutex.unlock();
}
template <class InputType, class OutputType>
bool ThreadPool<InputType, OutputType>::HasOutputFast()
{
    return outputQueue.IsEmpty()==false;
}
template <class InputType, class OutputType>
bool ThreadPool<InputType, OutputType>::HasOutput()
{
    bool res;
    outputQueueMutex.lock();
    res=outputQueue.IsEmpty()==false;
    outputQueueMutex.unlock();
    return res;
}
template <class InputType, class OutputType>
bool ThreadPool<InputType, OutputType>::HasInputFast()
{
    return inputQueue.IsEmpty()==false;
}
template <class InputType, class OutputType>
bool ThreadPool<InputType, OutputType>::HasInput()
{
    bool res;
    inputQueueMutex.lock();
    res=inputQueue.IsEmpty()==false;
    inputQueueMutex.unlock();
    return res;
}
template <class InputType, class OutputType>
OutputType ThreadPool<InputType, OutputType>::GetOutput()
{
    // Real output check
    OutputType output;
    outputQueueMutex.lock();
    output=outputQueue.Pop();
    outputQueueMutex.unlock();
    return output;
}
template <class InputType, class OutputType>
void ThreadPool<InputType, OutputType>::Clear()
{
    runThreadsMutex.lock();
    if (runThreads)
    {
        runThreadsMutex.unlock();
        inputQueueMutex.lock();
        inputFunctionQueue.Clear();
        inputQueue.Clear();
        inputQueueMutex.unlock();

        outputQueueMutex.lock();
        outputQueue.Clear();
        outputQueueMutex.unlock();
    }
    else
    {
        inputFunctionQueue.Clear();
        inputQueue.Clear();
        outputQueue.Clear();
    }
}
template <class InputType, class OutputType>
void ThreadPool<InputType, OutputType>::LockInput()
{
    inputQueueMutex.lock();
}
template <class InputType, class OutputType>
void ThreadPool<InputType, OutputType>::UnlockInput()
{
    inputQueueMutex.unlock();
}
template <class InputType, class OutputType>
unsigned ThreadPool<InputType, OutputType>::InputSize()
{
    return inputQueue.Size();
}
template <class InputType, class OutputType>
InputType ThreadPool<InputType, OutputType>::GetInputAtIndex(unsigned index)
{
    return inputQueue[index];
}
template <class InputType, class OutputType>
void ThreadPool<InputType, OutputType>::RemoveInputAtIndex(unsigned index)
{
    inputQueue.RemoveAtIndex(index);
    inputFunctionQueue.RemoveAtIndex(index);
}
template <class InputType, class OutputType>
void ThreadPool<InputType, OutputType>::LockOutput()
{
    outputQueueMutex.lock();
}
template <class InputType, class OutputType>
void ThreadPool<InputType, OutputType>::UnlockOutput()
{
    outputQueueMutex.unlock();
}
template <class InputType, class OutputType>
unsigned ThreadPool<InputType, OutputType>::OutputSize()
{
    return outputQueue.Size();
}
template <class InputType, class OutputType>
OutputType ThreadPool<InputType, OutputType>::GetOutputAtIndex(unsigned index)
{
    return outputQueue[index];
}
template <class InputType, class OutputType>
void ThreadPool<InputType, OutputType>::RemoveOutputAtIndex(unsigned index)
{
    outputQueue.RemoveAtIndex(index);
}
template <class InputType, class OutputType>
void ThreadPool<InputType, OutputType>::ClearInput()
{
    inputQueue.Clear();
    inputFunctionQueue.Clear();
}

template <class InputType, class OutputType>
void ThreadPool<InputType, OutputType>::ClearOutput()
{
    outputQueue.Clear();
}
template <class InputType, class OutputType>
bool ThreadPool<InputType, OutputType>::IsWorking()
{
    bool isWorking;
//    workingThreadCountMutex.lock();
//    isWorking=numThreadsWorking!=0;
//    workingThreadCountMutex.unlock();

//    if (isWorking)
//        return true;

    // Bug fix: Originally the order of these two was reversed.
    // It's possible with the thread timing that working could have been false, then it picks up the data in the other thread, then it checks
    // here and sees there is no data.  So it thinks the thread is not working when it was.
    if (HasOutputFast() && HasOutput())
        return true;

    if (HasInputFast() && HasInput())
        return true;

    // Need to check is working again, in case the thread was between the first and second checks
    workingThreadCountMutex.lock();
    isWorking=numThreadsWorking!=0;
    workingThreadCountMutex.unlock();

    return isWorking;
}

template <class InputType, class OutputType>
int ThreadPool<InputType, OutputType>::NumThreadsWorking()
{
    return numThreadsWorking;
}

template <class InputType, class OutputType>
bool ThreadPool<InputType, OutputType>::WasStarted()
{
    bool b;
    runThreadsMutex.lock();
    b = runThreads;
    runThreadsMutex.unlock();
    return b;
}
template <class InputType, class OutputType>
bool ThreadPool<InputType, OutputType>::Pause()
{
    if (WasStarted()==false)
        return false;

    workingThreadCountMutex.lock();
    while (numThreadsWorking>0)
    {
        RakSleep(30);
    }
    return true;
}
template <class InputType, class OutputType>
void ThreadPool<InputType, OutputType>::Resume()
{
    workingThreadCountMutex.unlock();
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif

