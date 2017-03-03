
#include "bro-config.h"

#include "Napatech.h"
#include <nt.h>

using namespace iosource::pktsrc;

NapatechSource::~NapatechSource()
{
	Close();
}

NapatechSource::NapatechSource(const std::string& path, bool is_live, const std::string& arg_kind)
{
	if ( ! is_live )
		Error("napatech source does not support offline input");

	kind = arg_kind;
	current_filter = -1;

	stream_id = atoi(path.c_str());

	props.path = path;
	props.is_live = is_live;

        status = NT_Init(NTAPI_VERSION);
        if ( status != NT_SUCCESS ) {
                NT_ExplainError(status, errorBuffer, sizeof(errorBuffer));
                Error(errorBuffer);
                return;
        }
}

static inline struct timeval nt_timestamp_to_timeval(const int64_t ts_nanosec)
{
    struct timeval tv;
    long tv_nsec;

    if (ts_nanosec == 0) {
        return (struct timeval) { 0, 0 };
    } else {
        tv.tv_sec = ts_nanosec / _NSEC_PER_SEC;
        tv_nsec = (ts_nanosec % _NSEC_PER_SEC);
        tv.tv_usec = tv_nsec / 1000;
    }

    return tv;
}

void NapatechSource::Open()
{
	// Last argument is the HBA - Host Buffer Allowance, or the amount of the Host Buffer that
	// can be held back. Need to createa  BIF to configure this.
	status = NT_NetRxOpen(&rx_stream, "BroStream", NT_NET_INTERFACE_PACKET, stream_id, 50);
	if ( status != NT_SUCCESS) {
		Info("Failed to open stream");
		NT_ExplainError(status, errorBuffer, sizeof(errorBuffer));
		Error(errorBuffer);
		return;
	}

	props.netmask = NETMASK_UNKNOWN;
	props.is_live = true;
	props.link_type = DLT_EN10MB;

	Opened(props);
}


void NapatechSource::Close()
{
	// Close configuration stream, release the host buffer, and remove all assigned NTPL assignments.
	NT_NetRxClose(rx_stream);
}

bool NapatechSource::ExtractNextPacket(Packet* pkt)
{
	u_char* data;
	while (true) {
		status = NT_NetRxGet(rx_stream, &packet_buffer, 1000);
		if (status != NT_SUCCESS) {
			if ((status == NT_STATUS_TIMEOUT) || (status == NT_STATUS_TRYAGAIN)) {
				// wait a little longer for a packet
				continue;
			}
			NT_ExplainError(status, errorBuffer, sizeof(errorBuffer));
			Info(errorBuffer);
			return false;
		}

		current_hdr.ts = nt_timestamp_to_timeval(NT_NET_GET_PKT_TIMESTAMP(packet_buffer));
		current_hdr.caplen = NT_NET_GET_PKT_CAP_LENGTH(packet_buffer);
		current_hdr.len = NT_NET_GET_PKT_WIRE_LENGTH(packet_buffer);
		data = (unsigned char *) NT_NET_GET_PKT_L2_PTR(packet_buffer);

		if ( ! ApplyBPFFilter(current_filter, &current_hdr, data) ) {
			DoneWithPacket();
			continue;
		}

		pkt->Init(props.link_type, &current_hdr.ts, current_hdr.caplen, current_hdr.len, data);
		++stats.received;
		return true;
	}

	// Should never reach this point
	return false;
}

void NapatechSource::DoneWithPacket()
{
	// release the current packet
	status = NT_NetRxRelease(rx_stream, packet_buffer);
	if (status != NT_SUCCESS) {
		NT_ExplainError(status, errorBuffer, sizeof(errorBuffer));
		Info(errorBuffer);
	}
}

bool NapatechSource::PrecompileFilter(int index, const std::string& filter)
{
    return PktSrc::PrecompileBPFFilter(index, filter);
}

bool NapatechSource::SetFilter(int index)
{
    current_filter = index;

    return true;
}

void NapatechSource::Statistics(Stats* s)
{
	s->received = stats.received;
	s->link = stats.received + num_discarded;
	// FIXME: Need to do calls to NTAPI to get drop counters
	s->dropped = 0;
}

iosource::PktSrc* NapatechSource::InstantiateNapatech(const std::string& path, bool is_live)
{
	return new NapatechSource(path, is_live, "napatech");
}


