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
    ThreadDataInterface() = default;
    virtual ~ThreadDataInterface() = default;

    virtual void *PerThreadFactory(void *context) = 0;
    virtual void PerThreadDestructor(void *factoryResult, void *context) = 0;
};

/// A simple class to create worker threads that processes a queue of functions with data.
/// This class does not allocate or deallocate memory.  It is up to the user to handle memory management.
/// InputType and OutputType are stored directly in a queue.  For large structures, if you plan to delete from the middle of the queue,
/// you might wish to store pointers rather than the structures themselves so the array can shift efficiently.
template<class InputType, class OutputType>
struct RAK_DLL_EXPORT ThreadPool
{
    ThreadPool();
    ~ThreadPool();

    using PerThreadInit = void *(*)();
    using PerThreadDeinit = void (*)(void *);
    using WorkerThreadCallback = OutputType (*)(InputType, bool *returnOutput, void *perThreadData);

    /// Start the specified number of threads.
    /// \param[in] numThreads The number of threads to start
    /// \param[in] _perThreadInit User callback to return data stored per thread.  Pass 0 if not needed.
    /// \param[in] _perThreadDeinit User callback to destroy data stored per thread, created by _perThreadInit.  Pass 0 if not needed.
    /// \return True on success, false on failure.
    bool StartThreads(int numThreads, PerThreadInit = nullptr, PerThreadDeinit = nullptr);

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
    void AddInput(WorkerThreadCallback workerThreadCallback, InputType inputData);

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
    RakNet::SimpleMutex inputQueueMutex, outputQueueMutex, workingThreadCountMutex, runThreadsMutex;

    PerThreadInit perThreadDataFactory;
    PerThreadDeinit perThreadDataDestructor;

    // inputFunctionQueue & inputQueue are paired arrays so if you delete from one at a particular index you must delete from the other
    // at the same index
    DataStructures::Queue<WorkerThreadCallback> inputFunctionQueue;
    DataStructures::Queue<InputType> inputQueue;
    DataStructures::Queue<OutputType> outputQueue;

    ThreadDataInterface *threadDataInterface;
    void *tdiContext;


    template<class ThreadInputType, class ThreadOutputType>
    friend RAK_THREAD_DECLARATION(WorkerThread);

    /// \internal
    bool runThreads;
    /// \internal
    std::atomic_int numThreadsRunning;
    /// \internal
    int numThreadsWorking;
    /// \internal

    RakNet::SignaledEvent quitAndIncomingDataEvents;
};

#include "impl/ThreadPool.impl"

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif

