#pragma once
#include "aoo.h"
#include "aoo_sink.hpp"
#include "aoo_client.hpp"
#include "common/net_utils.hpp"

#include <thread>
// #include "aoo_types.h"

#define DEFAULT_LATENCY 25
#define TD_LOG std::cout

#ifdef __APPLE__
	#include <iostream>
#else
	#include <stdio.h>
#endif

namespace TD
{
	/**
	 * @brief Aoo Receive Delegate
	 * 
	 */
	class AooReceive 
	{
	public:
		AooReceive(int32_t numChannels, double sr, int32_t numSamples, int32_t port, AooId id, int latency = DEFAULT_LATENCY);
		~AooReceive();

		void start();
		void stop();

		void process(float **outBuf, int numFrames);

		void handleEvent(const AooEvent &event);

		bool initialized() const { return initialized_; };
		// double getSampleRate() const { return static_cast<double>(sr); };
		// int getNumChannels() const { return numChannels_; };

		AooError setupSink(AooInt32 numChannels, AooSampleRate sr, AooInt32 blockSize, AooSetupFlags flags);
		std::string getAooInfo();
		std::string getAooError();
		
		private:
		AooSink::Ptr sink_;
		AooClient::Ptr client_;
		
		std::string aooError;
		std::string aooInfo;
		
		// int numChannels;
		// AooSampleRate sr;
		// AooInt32 blockSize;
		// int port;
		// AooId id;
		// int latency;

		
		bool initialized_ = false;

		std::thread send_thread_;
		std::thread receive_thread_;

	};
}
