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

int main()
{
    std::cout << "Initializing OpenStick ISOBUS Telemetry Node...\n";

    // 1. Assign a hardware tracking log layer
    isobus::CANStackLogger::set_log_level(isobus::CANStackLogger::LoggingLevel::Info);

    // 2. Map your physical SocketCAN hardware interface link (attached via UGREEN hub)
    std::shared_ptr<isobus::SocketCANInterface> canInterface = std::make_shared<isobus::SocketCANInterface>("can0");

    // 3. Define your unique 64-bit ISO 11783 NAME parameter block
    // This tells the tractor's ECU who your OpenStick gateway is
    isobus::NAME openStickName(0);
    openStickName.set_arbitrary_address_capable(true); // Must be true for dynamic claiming
    openStickName.set_industry_group(2);               // Agricultural and Forestry Equipment
    openStickName.set_device_class(0);                 // Non-specific task controller node
    openStickName.set_function_code(130);              // Telematics / Gateway node
    openStickName.set_manufacturer_code(1178);         // Open-source sandbox tracking value
    openStickName.set_identity_number(12345);          // Unique serial footprint tracking number

    // 4. Instantiate your Internal Control Function targeting a preferred starting address
    // 0x80 (128) is the standard industry starting block for external telemetry modules
    std::shared_ptr<isobus::InternalControlFunction> openStickICF = 
        isobus::CANNetworkManager::CANNetworkManager::create_internal_control_function(openStickName, 0x80, 0);

    // 5. Connect the hardware layer plugin to the protocol stack execution manager
    if (!isobus::CANHardwareInterface::get_instance().assign_can_channel_frame_handler(0, canInterface))
    {
        std::cerr << "⚠️ Critical: Failed to bind protocol stack to can0 hardware driver interface.\n";
        return -1;
    }

    // Start the underlying hardware threading queues
    if (!isobus::CANHardwareInterface::get_instance().start())
    {
        std::cerr << "⚠️ Critical: Failed to spin up internal hardware execution threads.\n";
        return -1;
    }

    std::cout << "✓ Address Claiming initialized. OpenStick is actively claiming address on the bus...\n";

    // 6. Primary operational execution loop
    while (true)
    {
        // Keep the address validation state machine alive and processing frame responses
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Check if our node has successfully secured an address on the bus topology
        if (openStickICF->get_is_valid())
        {
            static bool printedStatus = false;
            if (!printedStatus)
            {
                std::cout << "✓ Success! OpenStick safely claimed Source Address: " 
                          << static_cast<int>(openStickICF->get_address()) << " (0x" 
                          << std::hex << static_cast<int>(openStickICF->get_address()) << ")\n";
                printedStatus = true;
            }
            
            // Your custom logic to safely request proprietary PGNs or push ISO-XML metrics goes here!
        }
    }

    isobus::CANHardwareInterface::get_instance().stop();
    return 0;
}

