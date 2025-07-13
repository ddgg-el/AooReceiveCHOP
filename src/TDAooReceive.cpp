#include "TDAooReceive.h"
#include "cassert"

#include <sstream>

using namespace TD;

AooReceive::AooReceive(int32_t numChannels, double sr, int32_t numSamples, int32_t port, AooId id, int latency)
	// numChannels(numChannels), sr(sr), blockSize(numSamples), port(port), id(id), latency(latency)
{
	sink_ = AooSink::create(id);

	if (!sink_) {
		aooError = "Could not create AOO sink";
		return;
	}

	sink_->setEventHandler(
		[](void *user, const AooEvent *event, AooThreadLevel) {
			static_cast<AooReceive*>(user)->handleEvent(*event);
        }, this, kAooEventModeCallback);

	sink_->setLatency(latency*0.001);

	client_ = AooClient::create();

	if (!client_) {
		aooError = "Could not create an AOO client";
		return;
	}
	
	AooClientSettings settings;
	settings.portNumber = port;
	if(auto err = client_->setup(settings); err == kAooOk){
		TD_LOG << "Receiving on port " << port << " with ID " << id << "..." << std::endl;
	} else {
        if(err == kAooErrorSocket){
            aooError = "Socket Error: " + aoo::socket::strerror(aoo::socket::get_last_error());
        } else {
            aooError =  aoo_strerror(err);
        }
        return;
	}

	client_->addSink(sink_.get());
	AooError err = setupSink(numChannels, sr, numSamples, 0);
	if(err != kAooOk){
		initialized_ = false;	
	} else {
		initialized_ = true;
	}

}

AooReceive::~AooReceive() {
	assert(client_);
    if(initialized_){
        client_->stop();
    }


	if (send_thread_.joinable()) {
        send_thread_.join();
    }

    if (receive_thread_.joinable()) {
        receive_thread_.join();
    }
}

AooError AooReceive::setupSink(AooInt32 numChannels, AooSampleRate sr, AooInt32 blockSize, AooSetupFlags flags = 0)
{
	AooError err = sink_->setup(numChannels, sr, blockSize, flags);
	if(err == kAooErrorNone){
		send_thread_ = std::thread([this]() {
			client_->send(kAooInfinite);
		});
	
		receive_thread_ = std::thread([this]() {
			client_->receive(kAooInfinite);
		});
	} else {
		std::string msg;
		msg = aoo_strerror(err);
		aooError = "Sink setup error: " + msg;
	}

	return err;
} 

void AooReceive::start()
{
	;
}

void AooReceive::stop()
{
	;
}

void AooReceive::process(float **outBuf, int numFrames)
{
	AooNtpTime t = aoo_getCurrentNtpTime();
	sink_->process((AooSample**) outBuf, numFrames, t, nullptr, nullptr);

	client_->notify();
}

void AooReceive::handleEvent(const AooEvent &event)
{
	// TD_LOG << "Received event: " << std::to_string(event.type) << std::endl;

	switch (event.type)
	{
	case kAooEventSourceAdd:
    case kAooEventSourceRemove:
    case kAooEventStreamStart:
    case kAooEventStreamStop:
    case kAooEventStreamState:
    case kAooEventStreamTime:
    case kAooEventStreamLatency:
    case kAooEventFormatChange:
    case kAooEventInviteDecline:
    case kAooEventInviteTimeout:
    case kAooEventUninviteTimeout:
    case kAooEventBufferOverrun:
    case kAooEventBufferUnderrun:
    case kAooEventBlockDrop:
    case kAooEventBlockResend:
    case kAooEventBlockXRun:
    case kAooEventSourcePing:
	{
		// TD_LOG << "Handling event type: " << std::to_string(event.type) << std::endl;
		auto& ep = event.endpoint.endpoint;
		aoo::ip_address addr((const sockaddr*)ep.address, ep.addrlen);
		switch (event.type)
		{
		
			case kAooEventSourcePing:
			{
				auto& e = event.sourcePing;

				double delta1 = aoo_ntpTimeDuration(e.t1, e.t2) * 1000.0;
				double delta2 = aoo_ntpTimeDuration(e.t3, e.t4) * 1000.0;
				double total_rtt = aoo_ntpTimeDuration(e.t1, e.t4) * 1000.0;
				double network_rtt = total_rtt - aoo_ntpTimeDuration(e.t2, e.t3) * 1000.0;

				std::ostringstream ss;
				ss << "Source ping from " << addr.name() << ": "
					<< "delta1 = " << delta1 << " ms, "
					<< "delta2 = " << delta2 << " ms, "
					<< "total RTT = " << total_rtt << " ms, "
					<< "network RTT = " << network_rtt << " ms";
					
					aooInfo = ss.str();
				break;
			}
			case kAooEventStreamStart:
			{	
				std::ostringstream ss;
				//TODO: print actual address and not reference
				ss << "Start stream from source " << event.streamStop.endpoint.address << std::endl;
				aooInfo = ss.str();
				break;
			}
			case kAooEventStreamStop:
			{
				std::ostringstream ss;
				//TODO: print actual address and not reference
				ss << "Stop stream from source " << event.streamStop.endpoint.address << std::endl;
				aooInfo = ss.str();
				break;
			}
			case kAooEventFormatChange:
			{
				auto& f = *event.formatChange.format;
				std::ostringstream ss;
				ss << "format changed: '" << f.codecName << "' codec, "
						<< f.numChannels << " channels, " << f.sampleRate
						<< " Hz, " << f.blockSize << " samples" << std::endl;
				aooInfo = ss.str();
				break;
			}
			case kAooErrorSocket:
			{
				std::string msg;
				msg = aoo::socket::strerror(aoo::socket::get_last_error());
				aooError = "Socket error: " + msg;
				break;
			}
			case kAooEventBufferOverrun:
			{
				aooInfo = "Buffer overrun detected";
				break;
			}
			case kAooEventBufferUnderrun:
			{
				aooInfo = "Buffer underrun detected";
				break;
			}
			
			default:
				break;
		}
		break;
	}
	default:
		aooInfo = "Unkown event type";
		break;
	}
}

std::string AooReceive::getAooInfo()
{
	std::string info = aooInfo;
	aooInfo.clear();
	return info;
}

std::string AooReceive::getAooError()
{
	std::string error = aooError;
	aooError.clear();
	return error;
}