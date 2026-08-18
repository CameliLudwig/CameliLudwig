// DaqDiBridge.cpp — MSVC 编译的桥接程序, 读取 USB-5856 DI 数据输出到 stdout
// 编译 (Developer Command Prompt for VS):
//   cl /EHsc /I"D:\DAQNavi\Inc" DaqDiBridge.cpp /link /LIBPATH:"D:\DAQNavi\Inc" Automation.BDaq4.lib
// 用法:
//   DaqDiBridge.exe                     → 读一次所有端口, 输出 JSON
//   DaqDiBridge.exe --poll 1000         → 每秒轮询, 持续输出

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "bdaqctrl.h"
using namespace Automation::BDaq;

static void printPorts(InstantDiCtrl* ctrl) {
    int count = ctrl->getPortCount();
    if (count <= 0) { printf("{\"err\":\"no ports\"}\n"); return; }
    uint8* buf = (uint8*)malloc(count);
    ErrorCode ret = ctrl->Read(0, count, buf);
    if (BioFailed(ret)) { printf("{\"err\":\"read failed 0x%X\"}\n", ret); free(buf); return; }
    printf("{\"ports\":[");
    for (int i = 0; i < count; i++) {
        printf("%s%d", i ? "," : "", buf[i]);
    }
    printf("]}\n");
    fflush(stdout);
    free(buf);
}

int main(int argc, char* argv[]) {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    InstantDiCtrl* ctrl = InstantDiCtrl::Create();
    if (!ctrl) { printf("{\"err\":\"Create failed\"}\n"); return 1; }

    DeviceInformation devInfo(-1);
    ErrorCode ret = ctrl->setSelectedDevice(devInfo);
    if (BioFailed(ret)) {
        // try by name
        DeviceInformation devInfo2(L"USB-5856,BID#0");
        ret = ctrl->setSelectedDevice(devInfo2);
        if (BioFailed(ret)) {
            printf("{\"err\":\"no device 0x%X\"}\n", ret);
            ctrl->Dispose();
            CoUninitialize();
            return 1;
        }
    }

    int pollMs = 0;
    if (argc >= 3 && strcmp(argv[1], "--poll") == 0)
        pollMs = atoi(argv[2]);
    if (pollMs < 100) pollMs = 100;

    printf("{\"status\":\"ok\",\"ports\":%d}\n", ctrl->getPortCount());
    fflush(stdout);

    if (argc < 2) {
        // 单次读取
        printPorts(ctrl);
    } else {
        // 轮询模式
        while (1) {
            printPorts(ctrl);
            Sleep(pollMs);
        }
    }

    ctrl->Dispose();
    CoUninitialize();
    return 0;
}
