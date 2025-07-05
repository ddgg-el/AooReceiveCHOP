#include "TDAooReceive.h"
#include "cassert"

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
        }, this, kAooEventModePoll);

	sink_->setLatency(latency*0.001);

	client_ = AooClient::create();
	
	AooClientSettings settings;
	settings.portNumber = port;
	if(auto err = client_->setup(settings); err == kAooOk){
		TD_LOG << "Receiving on port " << port << " with ID " << id << "..." << std::endl;
	} else {
        if(err == kAooErrorSocket){
            aooError = "Socket Error: " + aoo::socket::strerror(aoo::socket::get_last_error());
        } else {
            aooError =  "Could not initialize AooClient";
        }
        return;
	}

	client_->addSink(sink_.get());
	setupSink(numChannels, sr, numSamples, 0);

	initialized_ = true;
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
	}

	return err;
} 

void AooReceive::start()
{
	// if (!source_host_.empty()) {
    //     // invite AOO source
    //     AooEndpoint ep = { &source_addr_, source_size_, source_id_ };

    //     std::cout << "invite source " << ep << "\n";

    //     assert(sink_);
    //     sink_->inviteSource(ep, nullptr);
    // }
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
	switch (event.type)
	{
	case kAooEventStreamStart:
	{
		TD_LOG << "Start stream from source " << event.streamStop.endpoint.address << std::endl;
		break;
	}
	case kAooEventStreamStop:
	{
		TD_LOG << "Stop stream from source " << event.streamStop.endpoint.address << std::endl;
		break;
	}
	case kAooEventFormatChange:
	{
		auto& f = *event.formatChange.format;
		std::cout << "format changed: '" << f.codecName << "' codec, "
		          << f.numChannels << " channels, " << f.sampleRate
		          << " Hz, " << f.blockSize << " samples" << std::endl;
		break;
	}
	case kAooErrorSocket:
	{
		std::string msg;
		msg = aoo::socket::strerror(aoo::socket::get_last_error());
		aooError = "Socket error: " + msg;
		break;
	}
	
	default:
		break;
	}
}
