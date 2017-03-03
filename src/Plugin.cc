#include "Plugin.h"
#include "Napatech.h"

namespace plugin { namespace Bro_Napatech { Plugin plugin; } }

using namespace plugin::Bro_Napatech;

plugin::Configuration Plugin::Configure()
{
    AddComponent(new ::iosource::PktSrcComponent("NapatechReader", "napatech", ::iosource::PktSrcComponent::LIVE, ::iosource::pktsrc::NapatechSource::InstantiateNapatech));

    plugin::Configuration config;
    config.name = "Bro::Napatech";
    config.description = "Packet acquisition via Napatech NTAPI";
    config.version.major = 1;
    config.version.minor = 0;

    return config;
}
