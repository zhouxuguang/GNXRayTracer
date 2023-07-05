//
//  Utils.cpp
//  GNXRayTracer
//
//  Created by zhouxuguang on 2023/7/5.
//

#include "Utils.h"
#include "RenderStatus.h"

#ifdef _WIN32


#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
void showMemoryInfo(void)
{

    //  SIZE_T PeakWorkingSetSize; //峰值内存使用
    //  SIZE_T WorkingSetSize; //内存使用
    //  SIZE_T PagefileUsage; //虚拟内存使用
    //  SIZE_T PeakPagefileUsage; //峰值虚拟内存使用

    EmptyWorkingSet(GetCurrentProcess());

    HANDLE handle = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));

    g_RenderStatus.setDataChanged("Memory Use", "WorkingSetSize", QString::number(pmc.WorkingSetSize / 1000.f / 1000.f), "M");
    g_RenderStatus.setDataChanged("Memory Use", "PeakWorkingSetSize", QString::number(pmc.PeakWorkingSetSize / 1000.f / 1000.f), "M");
    g_RenderStatus.setDataChanged("Memory Use", "PagefileUsage", QString::number(pmc.PagefileUsage / 1000.f / 1000.f), "M");
    g_RenderStatus.setDataChanged("Memory Use", "PeakPagefileUsage", QString::number(pmc.PeakPagefileUsage / 1000.f / 1000.f), "M");

}

#else

#import <mach/mach.h>

void showMemoryInfo(void)
{
    int64_t memoryUsageInByte = 0;
    task_vm_info_data_t vmInfo;
    mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
    kern_return_t kernelReturn = task_info(mach_task_self(), TASK_VM_INFO, (task_info_t) &vmInfo, &count);
    if(kernelReturn == KERN_SUCCESS)
    {
        memoryUsageInByte = (int64_t) vmInfo.phys_footprint;
        //NSLog(@"Memory in use (in bytes): %lld", memoryUsageInByte);
    }
    else
    {
        //NSLog(@"Error with task_info(): %s", mach_error_string(kernelReturn));
    }
    
    g_RenderStatus.setDataChanged("Memory Use", "WorkingSetSize", QString::number(vmInfo.resident_size / 1000.f / 1000.f), "M");
    g_RenderStatus.setDataChanged("Memory Use", "PeakWorkingSetSize", QString::number(vmInfo.resident_size_peak / 1000.f / 1000.f), "M");
    g_RenderStatus.setDataChanged("Memory Use", "PagefileUsage", QString::number(vmInfo.phys_footprint / 1000.f / 1000.f), "M");
    g_RenderStatus.setDataChanged("Memory Use", "PeakPagefileUsage", QString::number(vmInfo.phys_footprint / 1000.f / 1000.f), "M");
    //return memoryUsageInByte;
}

#endif // _WIN32
