#pragma once
#include "aoo.h"
#include "aoo_sink.hpp"
#include "aoo_client.hpp"

// #include "oscpack/osc/OscOutboundPacketStream.h"
#include "common/net_utils.hpp"

#define DEFAULT_LATENCY 25

namespace TD
{
	class AooClient;

	/**
	 * @brief Base class for AooNode
	 * 
	 */
	class INode {
	public:
		using ptr = std::shared_ptr<INode>;

		static INode::ptr get(int port);

		virtual ~INode() {}

		virtual int port() const = 0;
	
		virtual AooClient * client() = 0;
	
		virtual bool registerClient(AooClient *c) = 0;
	
		virtual void unregisterClient(AooClient *c) = 0;
	
		virtual void notify() = 0;
	
		// void sendReply(const osc::OutboundPacketStream& msg) {
		// 	sendReply(msg.Data(), msg.Size());
		// };

		virtual void sendReply(const char *data, size_t size) = 0;


	};

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

	class AooUnit;

	/**
	 * @brief Base Class for all Aoo Object interfaces
	 * 
	 */
	class AooDelegate : public std::enable_shared_from_this<AooDelegate> {
	public:
		AooDelegate(AooUnit& owner);
		~AooDelegate();

		AooUnit& unit() { return *owner_; }
		INode *node() { return node_.get(); };

		void setNode(std::shared_ptr<INode> node);

	protected:
		virtual void onDetach() = 0;

	private:
		AooUnit* owner_;
		std::shared_ptr<INode> node_;
	};

	/**
	 * @brief A generic Aoo object
	 * 
	 * maybe not needed
	 */
	class AooUnit {
	public:
		AooUnit();
		~AooUnit();

		bool initialized() const;
	protected:
		std::shared_ptr<AooDelegate> delegate_;
	};

	/**
	 * @brief Aoo interface for the TD CHOP
	 * 
	 */
	class AooReceive final : public AooDelegate {
	public:
		using AooDelegate::AooDelegate;

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