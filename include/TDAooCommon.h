#pragma once

#include "aoo.h"
#include "aoo_sink.hpp"
#include "aoo_client.hpp"

#include "common/net_utils.hpp"

#ifdef __APPLE__
	#include <iostream>
#else
	#include <stdio.h>
#endif

#include "CHOP_CPlusPlusBase.h"

#define DEFAULT_LATENCY 25
#define TD_LOG std::cout

#include "TDAooNode.h"

#if __APPLE__
	#include <iostream>
#else
	#include <stdio.h>
#endif

namespace TD
{

	class AooCHOP;
	/**
	 * @brief Base Class for all Aoo Object interfaces
	 * 
	 */
	class AooDelegate {
		public:
			AooDelegate(AooCHOP& owner);
			virtual ~AooDelegate(){};

			bool alive() const { return owner_ != nullptr; };
			bool initialized() const { return owner_ != nullptr; };

			void detach() {
				owner_ = nullptr;
				onDetach();
			};

			AooCHOP& unit() { return *owner_; }
			INode *node() { return node_.get(); };

			void setNode(std::shared_ptr<INode> node);

		protected:
			virtual void onDetach() = 0;

		private:
			AooCHOP* owner_;
			std::shared_ptr<INode> node_;
	};


	
	/**
	 * @brief A generic AooCHOP class
	 */
	class AooCHOP : public CHOP_CPlusPlusBase {
		public:
			AooCHOP(){};
			~AooCHOP(){
				aoo_delegate_->detach();
			};
	
			bool initialized() const;
			
		protected:
			std::unique_ptr<AooDelegate> aoo_delegate_;
		};
}