#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>

#include "isobus/isobus/can_network_manager.hpp"
#include "isobus/isobus/can_partnered_control_function.hpp"
#include "isobus/isobus/can_internal_control_function.hpp"
#include "isobus/hardware_integration/can_hardware_interface.hpp"
#include "isobus/hardware_integration/socket_can_interface.hpp"

#include "isobus/isobus/can_stack_logger.hpp"

/* #include "isobus/utility/can_stack_logger.hpp" */

int main()
{
    std::cout << "Initializing OpenStick ISOBUS Telemetry Node...\n";

    // 1. Assign a hardware tracking log layer
    isobus::CANStackLogger::set_log_level(isobus::CANStackLogger::LoggingLevel::Info);

    // 2. Map your physical SocketCAN hardware interface link
    std::shared_ptr<isobus::SocketCANInterface> canInterface = std::make_shared<isobus::SocketCANInterface>("can0");

    // 3. Define your unique 64-bit ISO 11783 NAME parameter block
    isobus::NAME openStickName(0);
    openStickName.set_arbitrary_address_capable(true); // Dynamic claiming fallback mode
    openStickName.set_industry_group(2);               // Agricultural Equipment
    openStickName.set_device_class(0);                 // Telematics Hub
    openStickName.set_function_code(130);              // Gateway code
    openStickName.set_manufacturer_code(1178);         // Sandbox identifier
    openStickName.set_identity_number(12345);          // Node Serial Number

    // 4. Corrected: Instantiate via the Network Manager Instance Singleton
/*
        isobus::CANNetworkManager::get_network_manager()->create_internal_control_function(openStickName, 0x80, 0);
std::shared_ptr<isobus::InternalControlFunction> openStickICF = 
        isobus::CANNetworkManager::CANNetworkManager::get_network_manager()->create_internal_control_function(openStickName, 0x80, 0);

std::shared_ptr<isobus::InternalControlFunction> openStickICF =
    isobus::CANNetworkManager::get_network_manager()->create_internal_control_function(openStickName, 0x80, 0);




std::shared_ptr<isobus::InternalControlFunction> openStickICF =
    isobus::CANNetworkManager::CANNetwork.create_internal_control_function(openStickName, 0x80, 0);


*/

/*
std::shared_ptr<isobus::InternalControlFunction> openStickICF =
    isobus::CANNetworkManager::CANNetwork->create_internal_control_function(openStickName, 0x80, 0);
*/


// To this exactly:
std::shared_ptr<isobus::InternalControlFunction> openStickICF =
    isobus::CANNetworkManager::CANNetwork.create_internal_control_function(openStickName, 0x80, 0);
    // 5. Corrected typo: assign handler through hardware singleton execution layer
    if (!isobus::CANHardwareInterface::assign_can_channel_frame_handler(0, canInterface))
    {
        std::cerr << "⚠️ Critical: Failed to bind protocol stack to can0 driver interface.\n";
        return -1;
    }

    // Corrected typo: Spin up native hardware threads
    if (!isobus::CANHardwareInterface::start())
    {
        std::cerr << "⚠️ Critical: Failed to spin up internal hardware execution threads.\n";
        return -1;
    }

    std::cout << "✓ Address Claiming initialized. OpenStick is actively claiming address on the bus...\n";

    // 6. Primary operational execution loop
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Corrected API parameter name based on compiler suggestion
        if (openStickICF->get_address_valid())
        {
            static bool printedStatus = false;
            if (!printedStatus)
            {
                std::cout << "✓ Success! OpenStick safely claimed Source Address: " 
                          << static_cast<int>(openStickICF->get_address()) << " (0x" 
                          << std::hex << static_cast<int>(openStickICF->get_address()) << ")\n";
                printedStatus = true;
            }
        }
    }

    // Corrected typo: Tear down interface
    isobus::CANHardwareInterface::stop();
    return 0;
}

