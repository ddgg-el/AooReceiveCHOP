/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "AooReceive_CHOP.h"
#include "aoo.h"

#ifdef __APPLE__
	#include <iostream>
#else
	#include <stdio.h>
#endif
#include <string.h>
#include <cmath>
#include <assert.h>

// These functions are basic C function, which the DLL loader can find
// much easier than finding a C++ Class.
// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
// you are creating
extern "C"
{

DLLEXPORT
void
FillCHOPPluginInfo(CHOP_PluginInfo *info)
{
	// Always set this to CHOPCPlusPlusAPIVersion.
	info->apiVersion = CHOPCPlusPlusAPIVersion;

	// The opType is the unique name for this CHOP. It must start with a 
	// capital A-Z character, and all the following characters must lower case
	// or numbers (a-z, 0-9)
	info->customOPInfo.opType->setString("Aooreceive");

	// The opLabel is the text that will show up in the OP Create Dialog
	info->customOPInfo.opLabel->setString("Aoo Receive");

	// Information about the author of this OP
	info->customOPInfo.authorName->setString("Davide Gagliardi");
	info->customOPInfo.authorEmail->setString("info@davidegagliardi.eu");

	// This CHOP can work with 0 inputs
	info->customOPInfo.minInputs = 0;

	// It can accept up to 1 input though, which changes it's behavior
	info->customOPInfo.maxInputs = 1;

}

DLLEXPORT
CHOP_CPlusPlusBase*
CreateCHOPInstance(const OP_NodeInfo* info)
{
	// AooError err = aoo_initialize(NULL);
	// if(err != kAooErrorNone){
	// 	// handle error
	// 	std::cout << "Error initializing Aoo: " << err << std::endl;
	// } else {
	// 	std::cout << "Aoo initialized" << std::endl;
	// }
	// Return a new instance of your class every time this is called.
	// It will be called once per CHOP that is using the .dll
	return new AooReceive_CHOP(info);
}

DLLEXPORT
void
DestroyCHOPInstance(CHOP_CPlusPlusBase* instance)
{
	// Delete the instance here, this will be called when
	// Touch is shutting down, when the CHOP using that instance is deleted, or
	// if the CHOP loads a different DLL
	delete (AooReceive_CHOP*)instance;
}

};


AooReceive_CHOP::AooReceive_CHOP(const OP_NodeInfo* info) : myNodeInfo(info)
{
	myExecuteCount = 0;
	myOffset = 0.0;

	std::cout << "Constructing AooReceive_CHOP" << std::endl;

}

AooReceive_CHOP::~AooReceive_CHOP()
{
	std::cout << "De-Constructing AooReceive_CHOP" << std::endl;
}

void
AooReceive_CHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
	std::cout << "Getting General Info" << std::endl;
	// This will cause the node to cook every frame
	ginfo->cookEveryFrameIfAsked = false;

	std::cout << inputs->getParInt("Numchannels") << std::endl;

	// Note: To disable timeslicing you'll need to turn this off, as well as ensure that
	// getOutputInfo() returns true, and likely also set the info->numSamples to how many
	// samples you want to generate for this CHOP. Otherwise it'll take on length of the
	// input CHOP, which may be timesliced.
	ginfo->timeslice = false;
	ginfo->inputMatchIndex = 0;
}

bool
AooReceive_CHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{
	// If there is an input connected, we are going to match it's channel names etc
	// otherwise we'll specify our own.
	if (inputs->getNumInputs() > 0)
	{
		return false;
	}
	else
	{
		info->numChannels = 1;

		// Since we are outputting a timeslice, the system will dictate
		// the numSamples and startIndex of the CHOP data
		//info->numSamples = 1;
		//info->startIndex = 0

		info->sampleRate = 44100;
		return true;
	}
}

void
AooReceive_CHOP::getChannelName(int32_t index, OP_String *name, const OP_Inputs* inputs, void* reserved1)
{
	name->setString("chan1");
}

void
AooReceive_CHOP::execute(CHOP_Output* output,
							  const OP_Inputs* inputs,
							  void* reserved)
{
	std::cout << "Executing AooReceive_CHOP" << std::endl;
	// auto sink = delegate().sink();
	// if (sink) {
	// 	uint64_t t = 0;//getOSCTime(mWorld);

	// 	if (sink->process(output->channels, output->numSamples, t, nullptr, nullptr) == kAooOk){
	// 		delegate().node()->notify();
	// 	} else {
	// 		// fill with zeros
	// 		std::fill(&output->channels[0][0], &output->channels[0][0] + output->numChannels * output->numSamples, 0.0f);
	// 	}
	// }
	// else {
		// fill with zeros
		std::fill(&output->channels[0][0], &output->channels[0][0] + output->numChannels * output->numSamples, 0.5f);
	// }
}


int32_t
AooReceive_CHOP::getNumInfoCHOPChans(void * reserved1)
{
	// We return the number of channel we want to output to any Info CHOP
	// connected to the CHOP. In this example we are just going to send one channel.
	return 2;
}

void
AooReceive_CHOP::getInfoCHOPChan(int32_t index,
										OP_InfoCHOPChan* chan,
										void* reserved1)
{
	// This function will be called once for each channel we said we'd want to return
	// In this example it'll only be called once.

	if (index == 0)
	{
		chan->name->setString("executeCount");
		chan->value = (float)myExecuteCount;
	}

	if (index == 1)
	{
		chan->name->setString("offset");
		chan->value = (float)myOffset;
	}
}

bool		
AooReceive_CHOP::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
{
	infoSize->rows = 2;
	infoSize->cols = 2;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void
AooReceive_CHOP::getInfoDATEntries(int32_t index,
										int32_t nEntries,
										OP_InfoDATEntries* entries, 
										void* reserved1)
{
	char tempBuffer[4096];

	if (index == 0)
	{
		// Set the value for the first column
		entries->values[0]->setString("executeCount");

		// Set the value for the second column
#ifdef _WIN32
		sprintf_s(tempBuffer, "%d", myExecuteCount);
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%d", myExecuteCount);
#endif
		entries->values[1]->setString(tempBuffer);
	}

	if (index == 1)
	{
		// Set the value for the first column
		entries->values[0]->setString("offset");

		// Set the value for the second column
#ifdef _WIN32
		sprintf_s(tempBuffer, "%g", myOffset);
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString( tempBuffer);
	}
}

// Called on Parameter changed
void
AooReceive_CHOP::buildDynamicMenu(const OP_Inputs* inputs, OP_BuildDynamicMenuInfo* info, void* reserved1)
{
//	double speed = inputs->getParDouble("Speed");
//	std::cout << "Speed: " << speed << std::endl;
	std::cout << "Changed Parameter: " << info->name << std::endl;

	if (!strcmp(info->name, "Shapemod"))
	{
		const char* shapeStr = inputs->getParString("Shape");
		if (!strcmp(shapeStr, "Sine"))
		{
			info->addMenuEntry("regular", "Regular");
			info->addMenuEntry("inverted", "Inverted");
		}
		else if (!strcmp(shapeStr, "Square"))
		{
			info->addMenuEntry("regular", "Regular");
			info->addMenuEntry("inverted", "Inverted");
		}
		else
		{
			info->addMenuEntry("increasing", "Increasing");
			info->addMenuEntry("decreasing", "Decreasing");
		}
	}
}

void
AooReceive_CHOP::setupParameters(OP_ParameterManager* manager, void *reserved1)
{
	std::cout << "Setup Parameters AooReceive_CHOP" << std::endl;

	// channels
	{
		OP_NumericParameter np;
		np.page = "Setup";
		np.name = "Numchannels";
		np.label = "Num Channels";
		np.defaultValues[0] = 0;
		np.minSliders[0] = 0;
		np.maxSliders[0] = 10;
		np.clampMins[0] = true;

		OP_ParAppendResult res = manager->appendInt(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// port
	{
		OP_NumericParameter np;
		np.page = "Setup";
		np.name = "Port";
		np.label = "Port";
		np.defaultValues[0] = 2;
		np.minSliders[0] = 0;
		np.maxSliders[0] = pow(2, 16) - 1; // 65535
		np.clampMins[0] = true;
		np.clampMaxes[0] = true;

		OP_ParAppendResult res = manager->appendInt(np);
		assert(res == OP_ParAppendResult::Success);
	}
	
	// id
	{
		OP_NumericParameter	np;
		np.page = "Setup";
		np.name = "Id";
		np.label = "Id";
		np.defaultValues[0] = 0;
		np.minSliders[0] = 0;
		np.maxSliders[0] =  10;
		np.clampMins[0] = true;
		
		OP_ParAppendResult res = manager->appendInt(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// latency
	{
		OP_NumericParameter	np;
		np.page = "Setup";
		np.name = "Latency";
		np.label = "Latency";
		np.defaultValues[0] = DEFAULT_LATENCY;
		np.minSliders[0] = 10;
		np.maxSliders[0] =  100;
		
		OP_ParAppendResult res = manager->appendInt(np);
		assert(res == OP_ParAppendResult::Success);
	}
}

void 
AooReceive_CHOP::pulsePressed(const char* name, void* reserved1)
{
	if (!strcmp(name, "Reset"))
	{
		myOffset = 0.0;
	}
}

void AooReceive_CHOP::handleParameters(const OP_Inputs* inputs, const OP_CHOPInput* chop)
{
	std::cout << "Handle Parameters AooReceive_CHOP" << std::endl;
	// speed
	// double speed = inputs->getParDouble("Speed");

	// // scale
	// double scale = inputs->getParDouble("Scale");

	// // shape
	// int shape = inputs->getParInt("Shape");

	// // shape modifier
	// const char* shapeMod = inputs->getParString("Shapemod");

	// // reset
	// if (inputs->getParInt("Reset"))
	// {
	// 	myOffset = 0.0;
	// }
}

void AooReceive_CHOP::setupSink()
{
	;
}

