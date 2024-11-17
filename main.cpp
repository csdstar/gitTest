#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <iomanip>  // 用于格式化输出

using namespace std;

const int indent = 35;

// 获取系统相关信息
void PrintSystemRelatedInfo() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&statex);

    // 使用setw来设置字段宽度，确保信息部分对齐
    cout << "---------系统相关信息-------------" << endl;
    cout << left << setw(indent - 2) << "虚拟分页大小:" << sysInfo.dwPageSize / 1024 << " KB" << endl;
    cout << left << setw(indent) << "最小应用程序大小:" << sysInfo.lpMinimumApplicationAddress << endl;
    cout << left << setw(indent) << "最大应用程序地址:" << sysInfo.lpMaximumApplicationAddress << endl;
    cout << left << setw(indent - 3) << "总虚拟内存:" << dec << (statex.ullTotalVirtual / (1024 * 1024 * 1024)) << " GB" << endl;
}

// 获取系统性能信息
void PrintSystemPerformanceInfo() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    // 页大小
    cout << "\n---------系统性能信息---------------" << endl;
    cout << left << setw(indent - 5) << "页大小:" << sysInfo.dwPageSize / 1024 << " KB" << endl;

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(MEMORYSTATUSEX);

    if (GlobalMemoryStatusEx(&statex)) {
        // 总物理内存
        cout << left << setw(indent - 3) << "总物理内存:" << fixed << setprecision(2) << static_cast<double>(statex.ullTotalPhys) / (1024 * 1024 * 1024) << " GB" << endl;
        // 空闲物理内存
        cout << left << setw(indent - 2) << "空闲物理内存:" << fixed << setprecision(2) << static_cast<double>(statex.ullAvailPhys) / (1024 * 1024 * 1024) << " GB" << endl;
    } else {
        cerr << "获取内存信息失败" << endl;
    }

    PERFORMANCE_INFORMATION perfInfo;
    if (GetPerformanceInfo(&perfInfo, sizeof(perfInfo))) {
        // 当前提交的页面总数
        cout << left << setw(indent + 1) << "当前提交的页面总数:" << perfInfo.CommitTotal << "页" << endl;
        // 当前可提交的最大页面数
        cout << left << setw(indent + 3) << "当前可提交的最大页面数:" << perfInfo.CommitLimit << "页" << endl;
        // 历史提交的页面峰值
        cout << left << setw(indent + 1) << "历史提交的页面峰值:" << perfInfo.CommitPeak << "页" << endl;
        // 当前打开的句柄数
        cout << left << setw(indent) << "当前打开的句柄数:" << perfInfo.HandleCount << endl;
        // 当前进程数
        cout << left << setw(indent - 3) << "当前进程数:" << perfInfo.ProcessCount << endl;
        // 当前线程数
        cout << left << setw(indent - 3) << "当前线程数:" << perfInfo.ThreadCount << endl;
    } else {
        cerr << "获取性能信息失败" << endl;
    }
}

int main() {
    PrintSystemRelatedInfo();
    PrintSystemPerformanceInfo();

    return 0;
}
