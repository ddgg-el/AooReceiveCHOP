#pragma once


#include "TDAooCommon.h"
// #include "oscpack/osc/OscOutboundPacketStream.h"


namespace TD
{


	

	/**
	 * @brief Node in the Aoo network
	 * 
	 */
	// class AooNode final : public INode {
	// public:
	// 	AooNode(int port);
	// 	~AooNode() override;

	// 	int port() const override { return port_; }

	// 	AooClient* client() override {
	// 		return client_.get();
	// 	}

	// 	bool registerClient(AooClient* c) override;

	// 	void unregisterClient(AooClient* c) override;

	// 	void notify() override {
	// 		client_->notify();
	// 	}
	// private:
	// 	int port_ = 0;
	// 	aoo::AooClient::Ptr client_;

	// };




	/**
	 * @brief Aoo Receive Delegate
	 * 
	 */
	class AooReceive final : public AooDelegate {
	public:
		using AooDelegate::AooDelegate;
		~AooReceive()
		{
			TD_LOG << "De-Constructing AooReceive Delegate" << std::endl;
		};

		void init(int32_t port, AooId id, AooSeconds latency);

		void onDetach() override;

		void handleEvent(const AooEvent *event);

		AooSink* sink() { return sink_.get(); }
	private:
		AooSink::Ptr sink_;
		int port = 0;
		AooId id = 0;
		AooSeconds latency = DEFAULT_LATENCY;

		int numChannels_ = 2;

	};
}