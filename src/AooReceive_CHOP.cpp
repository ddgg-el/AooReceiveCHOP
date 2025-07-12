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

#include "CHOP_CPlusPlusBase.h"
#include "AooReceive_CHOP.h"

#include <string.h>
#include <cmath>
#include <assert.h>

#include <Python.h>
#include <structmember.h>

static PyObject*
pyReset(PyObject* self)
{
	PY_Struct* me = (PY_Struct*)self;

	PY_GetInfo info;
	info.autoCook = false;
	AooReceive_CHOP* inst = (AooReceive_CHOP*)me->context->getNodeInstance(info);

	if(inst)
	{
		me->context->makeNodeDirty();
	}

	Py_INCREF(Py_None);
	return Py_None;
}

const char* PythonCallbackDATStubs =
"# me - this DAT\n"
"\n"
"# eventOp - The instance of the AooReceive_CHOP.\n"
"# info - Logs from the AooNetwork\n"
"#\n"
"def aoo_log(eventOp, info):\n"
"	# print(info)\n"
"	pass\n";

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
	info->customOPInfo.maxInputs = 0;

	info->customOPInfo.pythonVersion->setString(PY_VERSION);
	info->customOPInfo.pythonCallbacksDAT = PythonCallbackDATStubs;

}



DLLEXPORT
CHOP_CPlusPlusBase*
CreateCHOPInstance(const OP_NodeInfo* info)
{
	 AooError err = aoo_initialize(NULL);
	 if(err != kAooErrorNone){
	 	// handle error
	 	TD_LOG << "Error initializing Aoo: " << err << std::endl;
	 } else {
	 	TD_LOG << "Aoo initialized" << std::endl;
	 }
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

// TODO: how do I get the sample rate from the host?
AooReceive_CHOP::AooReceive_CHOP(const OP_NodeInfo* info) :
	num_channels_(2), port_(9009), id_(0), sr_(48000), nodeInfo(info)
{
	TD_LOG << "Constructing AooReceive_CHOP" << std::endl;
}

AooReceive_CHOP::~AooReceive_CHOP()
{
	TD_LOG << "De-Constructing AooReceive_CHOP" << std::endl;
	if(aoo_receiver != nullptr){
		delete aoo_receiver;
	}
}

void
AooReceive_CHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
	// This will cause the node to cook every frame
	ginfo->cookEveryFrameIfAsked = true;
	
	// Note: To disable timeslicing you'll need to turn this off, as well as ensure that
	// getOutputInfo() returns true, and likely also set the info->numSamples to how many
	// samples you want to generate for this CHOP. Otherwise it'll take on length of the
	// input CHOP, which may be timesliced.
	ginfo->timeslice = true;
	ginfo->inputMatchIndex = 0;
}

bool
AooReceive_CHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{
//	TD_LOG << "Getting Output Info" << std::endl;
	// If there is an input connected, we are going to match it's channel names etc
	// otherwise we'll specify our own.
	int32_t numchannels = inputs->getParInt("Numchannels");
	
    info->numChannels = numchannels;
	

	// Since we are outputting a timeslice, the system will dictate
	// the numSamples and startIndex of the CHOP data
	//info->numSamples = 1;
	//info->startIndex = 0

	info->sampleRate = sr_;
	return true;
	
}

void
AooReceive_CHOP::getChannelName(int32_t index, OP_String *name, const OP_Inputs* inputs, void* reserved1)
{
    std::string channelName = "chan" + std::to_string(index + 1);
    name->setString(channelName.c_str());
}

void
AooReceive_CHOP::execute(CHOP_Output* output,
							  const OP_Inputs* inputs,
							  void* reserved)
{
    int32_t port = inputs->getParInt("Port");
    int32_t id = inputs->getParInt("Id");
    int32_t latency = inputs->getParInt("Latency");
    int32_t numchannels = output->numChannels; // inputs->getParInt("Numchannels");
    
    if(num_channels_ != numchannels || output->numSamples != blockSize_ || port_ != port || id_ != id || latency_ != latency){
        num_channels_ = numchannels;
		blockSize_ = output->numSamples;
        port_ = port;
        id_ = id;
        latency_ = latency;
        
        errorString.clear();
        
        if(errorString.empty()){
            setupReceiver();
        }
    }
    
	if(aoo_receiver && aoo_receiver->initialized()){
		std::string aooInfo = aoo_receiver->getAooInfo();
		if(!aooInfo.empty()){
			PyObject* args = nodeInfo->context->createArgumentsTuple(1, nullptr);
			PyTuple_SET_ITEM(args, 1, PyUnicode_FromString(aooInfo.c_str()));
			PyObject* result = nodeInfo->context->callPythonCallback("aoo_log", args, nullptr, nullptr);
			Py_DECREF(args);
			TD_LOG << "Aoo Info: " << aooInfo << std::endl;
		}
		aoo_receiver->process(output->channels, output->numSamples);
	} else {
		// fill with zeros
		for (int i = 0; i < output->numChannels; ++i)
		{
			for (int j = 0; j < output->numSamples; ++j)
			{
				output->channels[i][j] = 0;
			}
	   }
	}
}


int32_t
AooReceive_CHOP::getNumInfoCHOPChans(void * reserved1)
{
	// We return the number of channel we want to output to any Info CHOP
	// connected to the CHOP. In this example we are just going to send one channel.
	return 4;
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
		// chan->name->setString("Channel Count");
		// chan->value = (float)num_channels_;
        chan->name->setString("Port");
        chan->value = (float)port_;
	}

	if (index == 1)
	{
        chan->name->setString("Channel Count");
        chan->value = (float)num_channels_;
		// chan->name->setString("Port");
		// chan->value = (float)port_;
	}

	if (index == 2)
	{
		chan->name->setString("ID");
		chan->value = (float)id_;
	}
	
	if (index == 3)
	{
		chan->name->setString("Latency");
		chan->value = (float)latency_;
	}
}

bool		
AooReceive_CHOP::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
{
	infoSize->rows = 2;
	infoSize->cols = 4;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = true;
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
		entries->values[0]->setString("channelCount");

		snprintf(tempBuffer, sizeof(tempBuffer), "%d", (int)num_channels_);
		entries->values[1]->setString(tempBuffer);
	}

	if (index == 1)
	{
		entries->values[0]->setString("port");
		snprintf(tempBuffer, sizeof(tempBuffer), "%d", (int)port_);
		entries->values[1]->setString( tempBuffer);
	}

	if (index == 2)
	{
		// Set the value for the first column
		entries->values[0]->setString("id");
		snprintf(tempBuffer, sizeof(tempBuffer), "%d", (int)id_);
		entries->values[1]->setString( tempBuffer);
	}

	if (index == 3)
	{
		// Set the value for the first column
		entries->values[0]->setString("latency");
		snprintf(tempBuffer, sizeof(tempBuffer), "%d", (int)latency_);
		entries->values[1]->setString( tempBuffer);
	}
}

void
AooReceive_CHOP::setupParameters(OP_ParameterManager* manager, void *reserved1)
{
	TD_LOG << "Setup Parameters AooReceive_CHOP" << std::endl;

	// channels
	{
		OP_NumericParameter np;
		np.page = "Setup";
		np.name = "Numchannels";
		np.label = "Num Channels";
		np.defaultValues[0] = 2;
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
		np.defaultValues[0] = 9009;
        np.minValues[0] = 0;
        np.maxValues[0] = 65535;
		np.minSliders[0] = 0;
        np.maxSliders[0] = 65535;
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
		// myOffset = 0.0;
	}
}

void
AooReceive_CHOP::getErrorString(OP_String *error, void *reserver1)
{
	error->setString(errorString.c_str());
//    errorString.clear();
//	aoo_receiver->aooError.clear();
}

void AooReceive_CHOP::setupReceiver()
{
	TD_LOG << "Setup AooReceiver" << std::endl;

	if(aoo_receiver != nullptr){
		delete(aoo_receiver);
	} 

	aoo_receiver = new AooReceive(num_channels_, sr_, blockSize_, port_, id_, latency_);
    
    errorString = aoo_receiver->getAooError();
}

