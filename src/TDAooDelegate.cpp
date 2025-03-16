#include "TDAooCommon.h"

using namespace TD;

AooDelegate::AooDelegate(AooCHOP& owner)
	: owner_(&owner)
{
	TD_LOG << "Constructing AooDelegate" << std::endl;;
}