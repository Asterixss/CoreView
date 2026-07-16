#include "Includes.h"
#include "Ex.h"

int main()
{
	Helpers::ClearConsole();

	Helpers::setWindowsConsole();
	design::enableVT();

	std::string banner = R"(
  ____             __     ___               
 / ___|___  _ __ __\ \   / (_) _____      __
| |   / _ \| '__/ _ \ \ / /| |/ _ \ \ /\ / /
| |__| (_) | | |  __/\ V / | |  __/\ V  V / 
 \____\___/|_|  \___| \_/  |_|\___| \_/\_/  
)";

	design::printRGB(banner, 0.0f);
	design::printRGB("\n[CoreView.2.1 - FR]\n", 0.0f);
	UserTime t = Helpers::GetCurrentTimeInfo();

	design::printRGB("[" + t.date + "] | [" + t.time + "] (" + t.timezone + ")\n\n", 0.0f);

	auto cpuResult = CPU::GatherCPUInfo();
	auto gpuResults = GPU::GatherGPUInfo();
	auto winResult = Win::GetOSInfo();
	auto netResults = Network::GetLocalAdapters();
	auto ramResult = RAM::GatherRAMInfo();

	design::setColor(3);
	std::cout << "-------- [CPU Information] --------\n";
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
	std::cout << "-------- [GPU Information] --------\n";
	std::cout << "[Name] " << Helpers::NarrowString(gpuResults[0].description) << "\n";
	std::cout << "[Dedicated Video Memory] " << gpuResults[0].dedicatedVideoMemory / (1024 * 1024) << " MB\n";
	std::cout << "[Vendor ID] " << gpuResults[0].vendorId << "\n";
	std::cout << "[Device ID] " << gpuResults[0].deviceId << "\n\n";

	design::setColor(6);
	std::cout << "-------- [RAM Information] --------\n";
	std::cout << "[Memory Load] " << ramResult.memoryLoadPercent << "%\n";
	std::cout << "[Total Physical RAM] " << (ramResult.totalPhysicalBytes / (1024 * 1024 * 1024)) << " GB\n";
	std::cout << "[Available RAM] " << (ramResult.availablePhysicalBytes / (1024 * 1024 * 1024)) << " GB\n";
	std::cout << "[Total Virtual Memory] " << (ramResult.totalVirtualBytes / (1024 * 1024 * 1024)) << " GB\n\n";

	int stickIndex = 1;
	for (const auto& stick : ramResult.sticks) {
		std::cout << "--- [RAM Stick #" << stickIndex++ << "] ---\n";
		std::cout << "[Manufacturer] " << Helpers::NarrowString(stick.manufacturer) << "\n";
		std::cout << "[Part Number] " << Helpers::NarrowString(stick.partNumber) << "\n";
		std::cout << "[Slot] " << Helpers::NarrowString(stick.bankLabel) << "\n";
		std::cout << "[Capacity] " << (stick.capacityBytes / (1024 * 1024 * 1024)) << " GB\n";
		std::cout << "[Speed] " << stick.speedMHz << " MHz\n";
		std::cout << "[Type] " << stick.memoryType << "\n";
		std::cout << "[Form Factor] " << stick.formFactor << "\n\n";
	}

	design::setColor(5);
	std::cout << "-------- [Storage Drive Information] --------\n";
	auto driveResults = Drive::GatherDriveInfo();
	for (const auto& drive : driveResults) {
		std::cout << "[Drive] " << Helpers::NarrowString(drive.letter) << "\n";
		std::cout << "[Volume Name] " << (drive.volumeName.empty() ? "Local Disk" : Helpers::NarrowString(drive.volumeName)) << "\n";
		std::cout << "[Drive Type] " << drive.driveType << "\n";
		std::cout << "[File System] " << Helpers::NarrowString(drive.fileSystem) << "\n";
		std::cout << "[Total Capacity] " << (drive.totalBytes / (1024 * 1024 * 1024)) << " GB\n";
		std::cout << "[Free Space] " << (drive.freeBytes / (1024 * 1024 * 1024)) << " GB\n\n";
	}

	design::setColor(11);
	std::cout << "-------- [Connected Peripherals] --------\n";
	auto peripheralResults = Peripheral::GatherConnectedDevices();
	for (const auto& dev : peripheralResults) {
		std::cout << "[Device Name] " << Helpers::NarrowString(dev.name) << "\n";
		std::cout << "[Category] " << Helpers::NarrowString(dev.category) << "\n";
		std::cout << "[Connection] " << dev.connectionType << "\n\n";
	}

	design::setColor(10);
	std::cout << "-------- [Network Information] --------\n";
	std::cout << "[Adapter Name] " << Helpers::NarrowString(netResults[0].adapterName) << "\n";
	std::cout << "[IP Address] " << netResults[0].ipAddress << "\n";
	if (netResults[0].isUp) { std::cout << "[Status] ✓\n\n"; }
	else { std::cout << "[Status] ✗\n\n"; }

	design::setColor(14);
	std::cout << "-------- [Windows Information] --------\n";
	std::cout << "[PC Name] " << Helpers::NarrowString(winResult.computerName) << "\n";

	std::cout << "[User Accounts] ";
	for (size_t i = 0; i < winResult.userAccounts.size(); ++i) {
		std::cout << Helpers::NarrowString(winResult.userAccounts[i]);
		if (i < winResult.userAccounts.size() - 1) {
			std::cout << ", ";
		}
	}
	std::cout << "\n";

	std::cout << "[Windows Architecture] " << winResult.osArchitecture << "\n";
	std::cout << "[Windows Edition] " << Helpers::NarrowString(winResult.osEdition) << "\n";
	std::cout << "[System Language] " << Helpers::NarrowString(winResult.systemLanguage) << "\n";
	if (winResult.batteryPercentage != -1) {
		std::cout << "[Battery Percentage] " << winResult.batteryPercentage << "%\n";
	}
	else {
		std::cout << "[Battery Percentage] No Battery found\n";
	}
	std::cout << "[Current Keyboard Language] " << Helpers::NarrowString(winResult.currentKeyboardLayout) << "\n";
	std::cout << "[Running Time] " << Helpers::NarrowString(winResult.upTime) << "\n\n";

	for (const auto& disp : winResult.displays) {
		std::cout << "[Monitor] " << Helpers::NarrowString(disp.deviceString) << "\n";
		std::cout << "[Resolution] " << disp.width << "x" << disp.height << "\n";
		std::cout << "[Refresh Rate] " << disp.refreshRate << " Hz\n\n";
	}

	design::printRGB("ESC to exit | ENTER to copy everything | SPACE to re-run | S to save to File", 0.0f);

	design::ScrollToConsoleTop();
	while (true) {
		int keyPressed = Helpers::WaitForKey();
		if (keyPressed == 27) {
			std::_Exit(0);
		}
		else if (keyPressed == 13) {
			Helpers::CopyConsoleToClipboard();
		}
		else if (keyPressed == 32) {
			Helpers::RestartApplication();
		}
		else if (keyPressed == 115 || keyPressed == 83) {
			std::wstring savedName;
			Helpers::SaveConsoleToFile(savedName);
			std::string narrowName(savedName.begin(), savedName.end());
		}
	}
}