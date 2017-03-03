#ifndef BRO_PLUGIN_BRO_NAPATECH
#define BRO_PLUGIN_BRO_NAPATECH

#include <plugin/Plugin.h>

namespace plugin {
namespace Bro_Napatech {

class Plugin : public ::plugin::Plugin
{
protected:
    // Overridden from plugin::Plugin.
    virtual plugin::Configuration Configure();
};

extern Plugin plugin;

}
}

#endif
