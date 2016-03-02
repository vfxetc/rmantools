#include "RixPattern.h"
#include "RixShadingUtils.h"
#include "RixColorUtils.h"
#include "RixIntegrator.h"
#include <cstring>
#include <stdint.h>
#include <stdio.h>
// #include <pthread.h>

#define DEBUG(x, ...) // printf("[KSRisAOVMulti] " x "\n", ##__VA_ARGS__); fflush(stdout);
    
#define NUM_INPUTS 10


class KSRisAOVMulti : public RixPattern
{
public:

    KSRisAOVMulti() {}
    virtual ~KSRisAOVMulti() {}
    virtual int Init(RixContext &, char const *pluginpath) { return 0; }
    virtual void Finalize(RixContext &) {}
    
    virtual RixSCParamInfo const *GetParamTable();
    
    virtual int CreateInstanceData(RixContext &ctx,
                                   char const *handle,
                                   RixParameterList const *plist,
                                   InstanceData *idata);

    virtual int ComputeOutputParams(RixShadingContext const *,
                                    RtInt *n, RixPattern::OutputSpec **outputs,
                                    RtConstPointer idata,
                                    RixSCParamInfo const *);

};


enum parameterID {
    k_resultAOV = 0,
    k_inputAOV,
    // We will dynamically add more here.
    k_colorOffset,
    k_nameOffset
};

 
RixSCParamInfo const *
KSRisAOVMulti::GetParamTable()
{
    //DEBUG("GetParamTable");
    static RixSCParamInfo s_ptable[] =
    {
        RixSCParamInfo("resultAOV", k_RixSCInteger, k_RixSCOutput),
        RixSCParamInfo("inputAOV", k_RixSCInteger),

        #define COLOR_PARAM_INFO(x) \
            RixSCParamInfo("color" #x, k_RixSCColor), \
            RixSCParamInfo("name" #x, k_RixSCString),
        COLOR_PARAM_INFO(1)
        COLOR_PARAM_INFO(2)
        COLOR_PARAM_INFO(3)
        COLOR_PARAM_INFO(4)
        COLOR_PARAM_INFO(5)
        COLOR_PARAM_INFO(6)
        COLOR_PARAM_INFO(7)
        COLOR_PARAM_INFO(8)
        COLOR_PARAM_INFO(9)
        COLOR_PARAM_INFO(10)

        RixSCParamInfo(), // end of table
    };
    return &s_ptable[0];
}


struct MyData {
public:
    int displayCount;
    int displays[NUM_INPUTS];
};

int
KSRisAOVMulti::CreateInstanceData(RixContext &ctx,
                               char const *handle,
                               RixParameterList const *plist,
                               InstanceData *idata)
{

    //DEBUG("CreateInstanceData");
    
    RtInt dataSize = sizeof(MyData);
    MyData *data = (MyData*)malloc(dataSize);
    data->displayCount = 0;
    
    idata->data = data;
    idata->datalen = dataSize;
    idata->freefunc = free;
    
    // Get the displays.
    RixRenderState *renderState;
    renderState = (RixRenderState *)ctx.GetRixInterface(k_RixRenderState);
    RixRenderState::FrameInfo frameInfo;
    renderState->GetFrameInfo(&frameInfo);
    RixIntegratorEnvironment *integratorEnv;
    integratorEnv = (RixIntegratorEnvironment*)frameInfo.integratorEnv;
    RtInt numDisplays = integratorEnv->numDisplays;
    RixDisplayChannel const* displayChannels = integratorEnv->displays;
    
    // Match them up to what was requested.
    for (int i = 0; i < NUM_INPUTS; i++) {
        
        data->displays[i] = -1;
        
        // Grab the requested AOV name.
        RtConstString name = NULL;
        plist->EvalParam(k_nameOffset + i * 2, 0, &name);
        if (!name || !strlen(name)) {
            continue;
        }
        
        DEBUG("name%d = \"%s\"", i + 1, name);
        
        RixSCType shadingType;
        RixSCConnectionInfo connInfo;
        plist->GetParamInfo(k_colorOffset + i * 2, &shadingType, &connInfo);
        if (connInfo == k_RixSCDefaultValue) {
            DEBUG("    nothing connected to input; skipping");
            continue;
        }
        
        // Find a matching display.
        for (int j = 0; j < numDisplays; j++) {
            if (!strcmp(displayChannels[j].channel, name)) {
                DEBUG("    display[%d] -> %d", j, displayChannels[j].id);
                data->displays[i] = displayChannels[j].id;
                data->displayCount++;
                break;
            }
        }
    
    }
    
    
    return 0;
}

int
KSRisAOVMulti::ComputeOutputParams(RixShadingContext const *sctx,
                                RtInt *noutputs, OutputSpec **outputs,
                                RtConstPointer instanceData,
                                RixSCParamInfo const *ignored)
{
    // Only execute on camera hits
    if (!(sctx->scTraits.primaryHit && sctx->scTraits.eyePath)) {
        return 1;
    }

    MyData const *data = (MyData const *)instanceData;
    
    // Pull on the inputAOV plug to make others evaluate.
    RtInt const *inputAOV = NULL;
    sctx->EvalParam(k_inputAOV, -1, &inputAOV);
    
    // Allocate outputs.
    RixShadingContext::Allocator pool(sctx);
    *noutputs = 1;
    OutputSpec* out = pool.AllocForPattern<OutputSpec>(*noutputs);
    *outputs = out;
    
    // Setup resultAOV plug for others to pull on.
    out->paramId = k_resultAOV;
    out->detail = k_RixSCUniform;
    out->value = (RtPointer)pool.AllocForPattern<RtInt>(1);
    ((RtInt*)out->value)[0] = inputAOV ? inputAOV[0] : 0;
        
    // If we don't have a display match, then just pass on through.
    // This needs to be "success" so that the values carry through.
    if (!data->displayCount) {
        return 0;
    }

    RixDisplayServices *displayServices = sctx->GetDisplayServices();
    
    for (int i = 0; i < NUM_INPUTS; i++) {
        
        // Don't bother pulling if there is no matching display.
        if (data->displays[i] < 0) {
            continue;
        }
        
        RtColorRGB const *inputColor;
        RixSCDetail inputDetail;
        inputDetail = sctx->EvalParam(k_colorOffset + i * 2, -1, &inputColor, NULL);
        if (!inputDetail) {
            continue;
        }

        for (int p = 0; p < sctx->numPts; p++) {
            // NOTE: We are using Write instead of Splat, which does not have any
            // filtering or anything.
            displayServices->Write(data->displays[i], sctx->integratorCtxIndex[p], inputColor[inputDetail == k_RixSCVarying ? p : 0]);
        }
    }
    
    return 0;
}


RIX_PATTERNCREATE
{
    return new KSRisAOVMulti();
}

RIX_PATTERNDESTROY
{
    delete ((KSRisAOVMulti*)pattern);
}

