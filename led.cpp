#include <boost/asio.hpp>
#include <gpiod.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <iostream>
#include <string>

namespace led
{

enum class Events
{
    on,
    off,
};

static bool setOutLed(gpiod::line& line, Events event)
{
    try
    {
        line.request({__FUNCTION__, gpiod::line_request::DIRECTION_OUTPUT},
                     static_cast<int>(event));
    }
    catch (const std::exception&)
    {
        std::cout << "Failed to request gpio " << line.name();
        return false;
    }

    return true;
}

}; // namespace led

int main(int argc, char** argv)
{
    using namespace led;

    boost::asio::io_service io;
    auto conn = std::make_shared<sdbusplus::asio::connection>(io);

    gpiod::line line = gpiod::find_line("UID_LED");
    if (!line)
    {
        std::cout << "Failed to find UID_LED GPIO line\n";
        return -1;
    }

    conn->request_name("xyz.openbmc_project.LED.GroupManager");

    sdbusplus::asio::object_server server =
        sdbusplus::asio::object_server(conn);
    std::shared_ptr<sdbusplus::asio::dbus_interface> iface;
    iface = server.add_interface(
        "/xyz/openbmc_project/led/groups/enclosure_identify",
        "xyz.openbmc_project.Led.Group");

    iface->register_property("Asserted", false,
                             [&line](const bool& req, bool& resp) {
                                 if (req)
                                 {
                                     system("gpioset gpiochip0 62=0");
                                     //  setOutLed(line, Events::on);
                                 }
                                 else if (!req)
                                 {
                                     system("gpioset gpiochip0 62=1");
                                     //  setOutLed(line, Events::off);
                                 }
                                 resp = req;
                                 return 1;
                             });

    iface->initialize();

    io.run();

    return 0;
}
