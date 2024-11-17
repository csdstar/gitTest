#include <windows.h>
#include <string>
#include <psapi.h>
#include <iostream>
#include <TlHelp32.h>
#include <iomanip>  // 用于格式化输出

using namespace std;

const int indent = 35;

// 获取系统相关信息
void printSystemRelatedInfo() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    

    // 使用setw来设置字段宽度，确保信息部分对齐
    cout << "\n--------------系统相关信息--------------" << endl;
    cout << left << setw(indent - 2) << "系统分页大小:" << sysInfo.dwPageSize / 1024 << " KB" << endl;
    cout << left << setw(indent) << "最小应用程序地址:" << sysInfo.lpMinimumApplicationAddress << endl;
    cout << left << setw(indent) << "最大应用程序地址:" << sysInfo.lpMaximumApplicationAddress << endl;
    cout << left << setw(indent - 1) << "处理器架构类型:";
    switch (sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_INTEL:
            cout << "x86" << endl;
            break;
        case PROCESSOR_ARCHITECTURE_AMD64:
            cout << "x64" << endl;
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            cout << "ARM" << endl;
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
            cout << "Itanium" << endl;
            break;
        case PROCESSOR_ARCHITECTURE_UNKNOWN:
        default:
            cout << "未知" << endl;
            break;
    }
    cout << left << setw(indent + 2) << "当前活跃的处理器数量:" << sysInfo.dwNumberOfProcessors << endl;;


    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&statex)) {
        // 总物理内存
        cout << left << setw(indent - 3) << "总物理内存:" << fixed << setprecision(2) << static_cast<double>(statex.ullTotalPhys) / (1024 * 1024 * 1024) << " GB" << endl;
        // 空闲物理内存
        cout << left << setw(indent - 2) << "空闲物理内存:" << fixed << setprecision(2) << static_cast<double>(statex.ullAvailPhys) / (1024 * 1024 * 1024) << " GB" << endl;
        // 总虚拟内存
        cout << left << setw(indent - 3) << "总虚拟内存:" << dec << fixed << setprecision(2) << static_cast<double>(statex.ullTotalVirtual / (1024 * 1024 * 1024)) << " GB" << endl;
        // 空闲虚拟内存
        cout << left << setw(indent - 2) << "空闲虚拟内存:" << dec << fixed << setprecision(2) << static_cast<double>(statex.ullAvailVirtual / (1024 * 1024 * 1024)) << " GB" << endl;
    } else {
        cerr << "获取内存信息失败" << endl;
    }
}

// 获取系统性能信息
void printSystemPerformanceInfo() {
    SYSTEM_INFO sysInfo;
    PERFORMANCE_INFORMATION perfInfo;
    GetSystemInfo(&sysInfo);

    // 页大小
    cout << "\n--------------系统性能信息--------------" << endl;
    cout << left << setw(indent - 5) << "页大小:" << sysInfo.dwPageSize / 1024 << " KB" << endl;

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

// 打印进程信息
void printProcessesInfo() {
    cout << "\n--------------系统进程信息--------------" << endl;
    // 获取当前系统的进程快照
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        cerr << "无法获取进程快照" << endl;
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // 获取第一个进程
    if (Process32First(hProcessSnap, &pe32)) {
        do {
            // 获取进程信息
            DWORD processID = pe32.th32ProcessID;               // 进程ID
            string processName = pe32.szExeFile;                // 进程名称

            // 打开进程句柄
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
            if (hProcess != NULL) {
                // 获取进程内存信息
                PROCESS_MEMORY_COUNTERS pmc;
                if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                    // 输出进程 ID、名称及使用的内存大小（单位：MB）
                    cout << left << setw(indent - 6) << "进程ID:" << processID << endl;
                    cout << left << setw(indent - 5) << "进程名:" << processName << endl;
                    cout << left << setw(indent - 2) << "进程占用内存:" <<  fixed << setprecision(2) << pmc.WorkingSetSize / (1024.0 * 1024.0) << " MB" << endl;
                    cout << endl;
                }
                CloseHandle(hProcess); // 关闭进程句柄
            }
        } while (Process32Next(hProcessSnap, &pe32)); // 遍历下一个进程
    } else {
        cerr << "无法获取进程信息" << endl;
    }

    CloseHandle(hProcessSnap); // 关闭进程快照句柄
}

// 查询进程信息
void queryProcessMemoryInfo(DWORD processID) {
    cout << "\n-----------------------查询进程虚拟地址信息-----------------------" << endl;
    cout << "正在查询进程ID: " << processID << "\n" << endl;
    
    // 获取进程句柄
    HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processID);
    if (hProcess == NULL) {
        cerr << "无法打开进程，错误代码: " << GetLastError() << endl;
        return;
    }

    cout << "查询到进程虚拟地址信息: "<<endl;
    cout << left << setw(15) << " 起始地址" << "  " <<  right << setw(15) << "终止地址 " << "(大小)    " << "区域状态   访问权限   内存类型" << endl;
    // 用于存储内存区域信息
    MEMORY_BASIC_INFORMATION mbi;
    LPVOID address = 0;  // 从地址 0 开始查询

    // 遍历进程的虚拟地址空间
    while (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)) == sizeof(mbi)) {
        // 获取起始地址、结束地址和区域大小
        DWORD64 startAddr = (DWORD64)mbi.BaseAddress;
        DWORD64 endAddr = (DWORD64)mbi.BaseAddress + mbi.RegionSize;
        double regionSizeMB = mbi.RegionSize / (1024.0 * 1024.0);  // 转换为MB

        // 输出信息
        cout << hex << setw(10) << setfill('0') << startAddr
             << " ~ "
             << setw(10) << setfill('0') << endAddr
             << " (" << fixed << setprecision(2) << regionSizeMB << "MB), ";

        // 显示区域状态
        switch (mbi.State) {
            case MEM_FREE:
                cout << "Free, ";
                break;
            case MEM_RESERVE:
                cout << "Reserved, ";
                break;
            case MEM_COMMIT:
                cout << "Committed, ";
                break;
            default:
                cout << "Unknown, ";
                break;
        }

        // 显示访问权限
        switch (mbi.Protect) {
            case PAGE_NOACCESS:
                cout << "NOACCESS, ";
                break;
            case PAGE_READONLY:
                cout << "READONLY, ";
                break;
            case PAGE_READWRITE:
                cout << "READWRITE, ";
                break;
            case PAGE_EXECUTE:
                cout << "EXECUTE, " ;
                break;
            case PAGE_EXECUTE_READ:
                cout << "EXECUTE_READ, ";
                break;
            case PAGE_EXECUTE_READWRITE:
                cout << "EXECUTE_READWRITE, " ;
                break;
            default:
                cout << "Other, ";
                break;
        }

        // 显示内存类型
        switch (mbi.Type) {
        case MEM_IMAGE:
            cout << "Image\n";
            break;
        case MEM_MAPPED:
            cout << "Mapped\n";
            break;
        case MEM_PRIVATE:
            cout << "Private\n";
            break;
        default:
            cout << "Unknown\n";
            break;
        }
        // 更新查询地址
        address = (LPBYTE)mbi.BaseAddress + mbi.RegionSize;
    }

    CloseHandle(hProcess);  // 关闭进程句柄
}

int main() {
    string input;
    DWORD processId;
    while (true) {
        // 提示用户输入命令
        cout << "\n请输入命令\n（'info' 查看系统信息，'performance' 查看性能信息，'process' 查看进程信息，'memory' 查询进程内存信息，'exit' 退出）: ";
        getline(cin, input);  // 获取用户输入的命令

        // 根据输入的命令调用对应的函数
        if (input == "info") {
            printSystemRelatedInfo();
        } 
        else if (input == "performance") {
            printSystemPerformanceInfo();
        }
        else if (input == "process") {
            printProcessesInfo();
        } 
        else if (input == "memory") {
            cout << "请输入进程ID: ";
            cin >> processId;  // 获取用户输入的进程ID
            cin.ignore();  // 忽略换行符
            queryProcessMemoryInfo(processId);
        } 
        else if (input == "exit") {
            cout << "退出程序..." << endl;
            break;  // 退出循环，结束程序
        } 
        else {
            cout << "无效命令，请重新输入。" << endl;
        }
    }
    return 0;
}
