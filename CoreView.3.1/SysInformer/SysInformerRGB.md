#include "Includes.h"
#include "Ex.h"
#include <cstdlib>
#include <ctime>

float getRand() { return static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 10.0f)); }

int main()
{
    Helpers::setWindowsConsole();
    design::enableVT();
    srand(static_cast<unsigned>(time(0)));

    

    design::printRGB(design::banner + "\n", getRand());
    design::printRGB("[CoreView.1 - FR]\n", getRand());

    UserTime t = Helpers::GetCurrentTimeInfo();
    design::printRGB("[" + t.date + "] | [" + t.time + "] (" + t.timezone + ")\n\n", getRand());

    auto cpuResult = CPU::GatherCPUInfo();
    auto gpuResults = GPU::GatherGPUInfo();
    auto winResult = Win::GetOSInfo();
    auto netResults = Network::GetLocalAdapters();
    auto ramResult = RAM::GatherRAMInfo();

    design::printRGB("-------- CPU Information--------\n", getRand());
    design::printRGB("[Name] " + cpuResult->brand + "\n", getRand());
    design::printRGB("[Brand] " + cpuResult->brand + "\n", getRand());
    design::printRGB("[Vendor] " + cpuResult->vendor + "\n", getRand());
    design::printRGB("[Architecture] " + cpuResult->arch + "\n", getRand());
    design::printRGB("[Physical Cores] " + std::to_string(cpuResult->physicalCores) + "\n", getRand());
    design::printRGB("[Logical Processors] " + std::to_string(cpuResult->logicalProcessors) + "\n", getRand());
    design::printRGB("[Max Clock Speed] " + std::to_string(cpuResult->maxClockSpeedMHz) + " MHz\n", getRand());
    design::printRGB(cpuResult->isHyperThreading ? "[Hyper-Threading] Enabled\n" : "[Hyper-Threading] Disabled\n\n", getRand());
    design::printRGB("[Sockets] " + std::to_string(winResult.sockets) + "\n", getRand());
    design::printRGB("[Cores] " + std::to_string(winResult.cores) + "\n", getRand());
    design::printRGB(winResult.virtualizationEnabled ? "[Virtualization] Enabled\n" : "[Virtualization] Disabled\n", getRand());
    design::printRGB("[l1CacheSizeKB] " + std::to_string(winResult.l1CacheSizeKB) + " KB\n", getRand());
    design::printRGB("[l2CacheSizeKB] " + std::to_string(winResult.l2CacheSizeKB) + " KB\n", getRand());
    design::printRGB("[l3CacheSizeKB] " + std::to_string(winResult.l3CacheSizeKB) + " KB\n\n", getRand());

    design::printRGB("-------- GPU Information--------\n", getRand());
    design::printRGB("[Name] " + Helpers::NarrowString(gpuResults[0].description) + "\n", getRand());
    design::printRGB("[Dedicated Video Memory] " + std::to_string(gpuResults[0].dedicatedVideoMemory / (1024 * 1024)) + " MB\n", getRand());
    design::printRGB("[Vendor ID] " + std::to_string(gpuResults[0].vendorId) + "\n", getRand());
    design::printRGB("[Device ID] " + std::to_string(gpuResults[0].deviceId) + "\n\n", getRand());

    design::printRGB("-------- RAM Information --------\n", getRand());
    design::printRGB("[Memory Load] " + std::to_string(ramResult.memoryLoadPercent) + "%\n", getRand());
    design::printRGB("[Total Physical RAM] " + std::to_string(ramResult.totalPhysicalBytes / (1024 * 1024 * 1024)) + " GB\n", getRand());
    design::printRGB("[Available RAM] " + std::to_string(ramResult.availablePhysicalBytes / (1024 * 1024 * 1024)) + " GB\n", getRand());
    design::printRGB("[Total Virtual Memory] " + std::to_string(ramResult.totalVirtualBytes / (1024 * 1024 * 1024)) + " GB\n\n", getRand());

    int stickIndex = 1;
    for (const auto& stick : ramResult.sticks) {
        design::printRGB("--- RAM Stick #" + std::to_string(stickIndex++) + " ---\n", getRand());
        design::printRGB("[Manufacturer] " + Helpers::NarrowString(stick.manufacturer) + "\n", getRand());
        design::printRGB("[Part Number] " + Helpers::NarrowString(stick.partNumber) + "\n", getRand());
        design::printRGB("[Slot] " + Helpers::NarrowString(stick.bankLabel) + "\n", getRand());
        design::printRGB("[Capacity] " + std::to_string(stick.capacityBytes / (1024 * 1024 * 1024)) + " GB\n", getRand());
        design::printRGB("[Speed] " + std::to_string(stick.speedMHz) + " MHz\n", getRand());
        design::printRGB("[Type] " + stick.memoryType + "\n", getRand());
        design::printRGB("[Form Factor] " + stick.formFactor + "\n\n", getRand());
    }

    design::printRGB("-------- Storage Drive Information --------\n", getRand());
    auto driveResults = Drive::GatherDriveInfo();
    for (const auto& drive : driveResults) {
        design::printRGB("[Drive] " + Helpers::NarrowString(drive.letter) + "\n", getRand());
        design::printRGB("[Volume Name] " + (drive.volumeName.empty() ? "Local Disk" : Helpers::NarrowString(drive.volumeName)) + "\n", getRand());
        design::printRGB("[Drive Type] " + drive.driveType + "\n", getRand());
        design::printRGB("[File System] " + Helpers::NarrowString(drive.fileSystem) + "\n", getRand());
        design::printRGB("[Total Capacity] " + std::to_string(drive.totalBytes / (1024 * 1024 * 1024)) + " GB\n", getRand());
        design::printRGB("[Free Space] " + std::to_string(drive.freeBytes / (1024 * 1024 * 1024)) + " GB\n\n", getRand());
    }

    design::printRGB("-------- Connected Peripherals --------\n", getRand());
    auto peripheralResults = Peripheral::GatherConnectedDevices();
    for (const auto& dev : peripheralResults) {
        design::printRGB("[Device Name] " + Helpers::NarrowString(dev.name) + "\n", getRand());
        design::printRGB("[Category] " + Helpers::NarrowString(dev.category) + "\n", getRand());
        design::printRGB("[Connection] " + dev.connectionType + "\n\n", getRand());
    }

    design::printRGB("-------- Network Information--------\n", getRand());
    design::printRGB("[Adapter Name] " + Helpers::NarrowString(netResults[0].adapterName) + "\n", getRand());
    design::printRGB("[IP Address] " + netResults[0].ipAddress + "\n", getRand());
    design::printRGB(netResults[0].isUp ? "[Status] Up\n\n" : "[Status] Down\n\n", getRand());

    design::printRGB("-------- Windows Information--------\n", getRand());
    design::printRGB("[PC Name] " + Helpers::NarrowString(winResult.computerName) + "\n", getRand());
    design::printRGB("[Windows Architecture] " + winResult.osArchitecture + "\n", getRand());
    design::printRGB("[Windows Edition] " + Helpers::NarrowString(winResult.osEdition) + "\n", getRand());
    design::printRGB("[System Language] " + Helpers::NarrowString(winResult.systemLanguage) + "\n", getRand());
    design::printRGB("[Running Time] " + Helpers::NarrowString(winResult.upTime) + "\n\n", getRand());
    design::printRGB("Press any key to exit...", getRand());

    system("pause > nul");
}