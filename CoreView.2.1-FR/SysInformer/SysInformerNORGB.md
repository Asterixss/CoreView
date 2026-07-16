#include "Includes.h"
#include "Ex.h"

int main()
{
	Helpers::setWindowsConsole();

	design::setColor(9);
    std::string banner = R"(
  ____             __     ___               
 / ___|___  _ __ __\ \   / (_) _____      __
| |   / _ \| '__/ _ \ \ / /| |/ _ \ \ /\ / /
| |__| (_) | | |  __/\ V / | |  __/\ V  V / 
 \____\___/|_|  \___| \_/  |_|\___| \_/\_/  
)";

    std::cout << banner << "\n";
    std::cout << "[CoreView.1 - FR]\n";
	UserTime t = Helpers::GetCurrentTimeInfo();

	std::cout << "[" << t.date << "] | [" << t.time << "] ("
		<< t.timezone << ")" << "\n\n";
    
    auto cpuResult = CPU::GatherCPUInfo();
	auto gpuResults = GPU::GatherGPUInfo();
	auto winResult = Win::GetOSInfo();
	auto netResults = Network::GetLocalAdapters();
	auto ramResult = RAM::GatherRAMInfo();

	design::setColor(3);
	std::cout << "-------- CPU Information--------\n";
	std::cout << "[Name] " << cpuResult->brand << "\n";
	std::cout << "[Brand] " << cpuResult->brand << "\n";
	std::cout << "[Vendor] " << cpuResult->vendor << "\n";
	std::cout << "[Architecture] " << cpuResult->arch << "\n";
    std::cout << "[Physical Cores] " << cpuResult->physicalCores << "\n";
	std::cout << "[Logical Processors] " << cpuResult->logicalProcessors << "\n";
	std::cout << "[Max Clock Speed] " << cpuResult->maxClockSpeedMHz << " MHz\n";
	if (cpuResult->isHyperThreading) { std::cout << "[Hyper-Threading] Enabled\n"; }
	else { std::cout << "[Hyper-Threading] Disabled\n\n"; }
	std::cout << "[Sockets] " << winResult.sockets << "\n";
	std::cout << "[Cores] " << winResult.cores << "\n";
	if (winResult.virtualizationEnabled) { std::cout << "[Virtualization] Enabled\n"; }
	else { std::cout << "[Virtualization] Disabled\n"; }
	std::cout << "[l1CacheSizeKB] " << winResult.l1CacheSizeKB << " KB\n";
	std::cout << "[l2CacheSizeKB] " << winResult.l2CacheSizeKB << " KB\n";
	std::cout << "[l3CacheSizeKB] " << winResult.l3CacheSizeKB << " KB\n\n";

	design::setColor(4);
	std::cout << "-------- GPU Information--------\n";
	std::cout << "[Name] " << Helpers::NarrowString(gpuResults[0].description) << "\n";
	std::cout << "[Dedicated Video Memory] " << gpuResults[0].dedicatedVideoMemory / (1024 * 1024) << " MB\n";
	std::cout << "[Vendor ID] " << gpuResults[0].vendorId << "\n";
	std::cout << "[Device ID] " << gpuResults[0].deviceId << "\n\n";

	design::setColor(6);
	std::cout << "-------- RAM Information --------\n";
	std::cout << "[Memory Load] " << ramResult.memoryLoadPercent << "%\n";
	std::cout << "[Total Physical RAM] " << (ramResult.totalPhysicalBytes / (1024 * 1024 * 1024)) << " GB\n";
	std::cout << "[Available RAM] " << (ramResult.availablePhysicalBytes / (1024 * 1024 * 1024)) << " GB\n";
	std::cout << "[Total Virtual Memory] " << (ramResult.totalVirtualBytes / (1024 * 1024 * 1024)) << " GB\n\n";

	int stickIndex = 1;
	for (const auto& stick : ramResult.sticks) {
		std::cout << "--- RAM Stick #" << stickIndex++ << " ---\n";
		std::cout << "[Manufacturer] " << Helpers::NarrowString(stick.manufacturer) << "\n";
		std::cout << "[Part Number] " << Helpers::NarrowString(stick.partNumber) << "\n";
		std::cout << "[Slot] " << Helpers::NarrowString(stick.bankLabel) << "\n";
		std::cout << "[Capacity] " << (stick.capacityBytes / (1024 * 1024 * 1024)) << " GB\n";
		std::cout << "[Speed] " << stick.speedMHz << " MHz\n";
		std::cout << "[Type] " << stick.memoryType << "\n";
		std::cout << "[Form Factor] " << stick.formFactor << "\n\n";
	}

	design::setColor(5);
	std::cout << "-------- Storage Drive Information --------\n";
	auto driveResults = Drive::GatherDriveInfo();
	for (const auto& drive : driveResults) {
		std::cout << "[Drive] " << Helpers::NarrowString(drive.letter) << "\n";
		std::cout << "[Volume Name] " << (drive.volumeName.empty() ? "Local Disk" : Helpers::NarrowString(drive.volumeName)) << "\n";
		std::cout << "[Drive Type] " << drive.driveType << "\n";
		std::cout << "[File System] " << Helpers::NarrowString(drive.fileSystem) << "\n";
		std::cout << "[Total Capacity] " << (drive.totalBytes / (1024 * 1024 * 1024)) << " GB\n";
		std::cout << "[Free Space] " << (drive.freeBytes / (1024 * 1024 * 1024)) << " GB\n\n";
	}

	design::setColor(7);
	std::cout << "-------- Connected Peripherals --------\n";
	auto peripheralResults = Peripheral::GatherConnectedDevices();
	for (const auto& dev : peripheralResults) {
		std::cout << "[Device Name] " << Helpers::NarrowString(dev.name) << "\n";
		std::cout << "[Category] " << Helpers::NarrowString(dev.category) << "\n";
		std::cout << "[Connection] " << dev.connectionType << "\n\n";
	}

	design::setColor(10);
	std::cout << "-------- Network Information--------\n";
	std::cout << "[Adapter Name] " << Helpers::NarrowString(netResults[0].adapterName) << "\n";
	std::cout << "[IP Address] " << netResults[0].ipAddress << "\n";
	if (netResults[0].isUp) { std::cout << "[Status] Up\n\n"; }
	else { std::cout << "[Status] Down\n\n"; }

	design::setColor(14);
	std::cout << "-------- Windows Information--------\n";
	std::cout << "[PC Name] " << Helpers::NarrowString(winResult.computerName) << "\n";
	std::cout << "[Windows Architecture] " << winResult.osArchitecture << "\n";
	std::cout << "[Windows Edition] " << Helpers::NarrowString(winResult.osEdition) << "\n";
	std::cout << "[System Language] " << Helpers::NarrowString(winResult.systemLanguage) << "\n";
	std::cout << "[Running Time] " << Helpers::NarrowString(winResult.upTime) << "\n\n";
	std::cout << "Press any key to exit...";
	system("pause > nul");
}