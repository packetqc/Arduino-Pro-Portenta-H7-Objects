#ifndef PORTENTA_PERF_H
#define PORTENTA_PERF_H


#include "mbed.h"
#include "mbed_stats.h"

//===================================================================================================
// PERFORMANCE WORKER
//===================================================================================================
using namespace mbed;
using namespace rtos;


//===================================================================================================
// PERFORMANCE
//===================================================================================================
class Performance {
    public:
        //SINGLETON
        static Performance &getInstance();    // Accessor for singleton instance
        Performance(const Performance &) = delete;  // no copying
        Performance &operator=(const Performance &) = delete;
        
        //THREAD START CALLBACK
        static void Run_callback();


    private:
        Performance() = default;  // Make constructor private  
        // Thread Performance_worker;

    public:
        bool runit = true;
        int pause = 60000;
        
        void Run(void);
        void HeapPerformance(void);
        void StackPerformance(void);
        void ThreadPerformance(void);
        void CpuPerformance(void);
};


extern Performance &mpPERF;

#endif