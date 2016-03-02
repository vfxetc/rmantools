#include "RixPattern.h"
#include "RixShadingUtils.h"
#include "RixColorUtils.h"
#include "RixIntegrator.h"
#include <cstring>
#include <stdint.h>
#include <stdio.h>


// Macro for applying another macro across our prefixes.
#define DEBUG(x, ...) printf("[KSRisAOVMulti] " x "\n", ##__VA_ARGS__); fflush(stdout);

#define MULTI_APPLY(macro, ...) \
    macro(1, ##__VA_ARGS__) \
    macro(2, ##__VA_ARGS__) \
    macro(3, ##__VA_ARGS__) \
    macro(4, ##__VA_ARGS__) \
    macro(5, ##__VA_ARGS__) \
    macro(6, ##__VA_ARGS__) \
    macro(7, ##__VA_ARGS__) \
    macro(8, ##__VA_ARGS__) \
    macro(9, ##__VA_ARGS__) \
    macro(10, ##__VA_ARGS__)
    
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
    k_colorOffset,
    k_nameOffset
    // We will dynamically add more here.
};

                                                   
RixSCParamInfo const *
KSRisAOVMulti::GetParamTable()
{
    static RixSCParamInfo s_ptable[] =
    {
        RixSCParamInfo("resultAOV", k_RixSCInteger, k_RixSCOutput),
        RixSCParamInfo("inputAOV", k_RixSCInteger),

        #define COLOR_PARAM_INFO(x) \
            RixSCParamInfo("color" #x, k_RixSCColor), \
            RixSCParamInfo("name" #x, k_RixSCString),
        // MULTI_APPLY(COLOR_PARAM_INFO)
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
    bool inited;
    int displayCount;
    int displays[NUM_INPUTS];
};

int
KSRisAOVMulti::CreateInstanceData(RixContext &ctx,
                               char const *handle,
                               RixParameterList const *plist,
                               InstanceData *idata)
{

    RtInt dataSize = sizeof(MyData*);
    MyData *data = (MyData*)malloc(dataSize);
    data->inited = false;
    data->displayCount = 0;
    for (int i = 0; i < NUM_INPUTS; i++) {
        data->displays[i] = -1;
    }
    idata->data = data;
    idata->datalen = dataSize;
    idata->freefunc = free;
    
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

    MyData *data = (MyData*)instanceData;
    if (!data->inited) {
                
        // Get the displays.
        RixRenderState *renderState;
        renderState = (RixRenderState *)sctx->GetRixInterface(k_RixRenderState);
        RixRenderState::FrameInfo frameInfo;
        renderState->GetFrameInfo(&frameInfo);
        RixIntegratorEnvironment *integratorEnv;
        integratorEnv = (RixIntegratorEnvironment*)frameInfo.integratorEnv;
        RtInt numDisplays = integratorEnv->numDisplays;
        RixDisplayChannel const* displayChannels = integratorEnv->displays;

        // Match them up to what was requested.
        for (int i = 0; i < NUM_INPUTS; i++) {
                        
            // Grab the requested AOV name.
            RtConstString *name;
            sctx->EvalParam(k_nameOffset + i * 2, -1, &name, NULL, false);
            if (!name || !strlen(*name)) {
                continue;
            }
            
            DEBUG("name%d = \"%s\"", i, *name);
            
            // Find a matching display.
            for (int j = 0; j < numDisplays; j++) {
                if (!strcmp(displayChannels[j].channel, *name)) {
                    DEBUG("     display[%d] -> %d", j, displayChannels[j].id);
                    data->displays[i] = displayChannels[j].id;
                    data->displayCount++;
                    break;
                }
            }
        
        }
        
        // At the end in case there are any threading problems.
        data->inited = true;
        
    }
    
    
    // Evaluate all inputs.
    RtColorRGB const *inputColor[NUM_INPUTS];
    bool inputExists[NUM_INPUTS];
    for (int i = 0; i < NUM_INPUTS; i++) {
        
        // Don't bother pulling if there is no matching display.
        if (data->displays[i] < 0) {
            inputExists[i] = false;
            continue;
        }
        
        RtColorRGB const *theColor = NULL;
        DEBUG("HERE 1.%d.1", i);
        bool exists = sctx->EvalParam(k_colorOffset + i * 2, -1, &theColor, NULL, true);
        DEBUG("HERE 1.%d.2", i);
        if (theColor && exists) {
            inputExists[i] = true;
            inputColor[i] = theColor;
        }
    }
        
    // Pull on the inputAOV plug to make others evaluate.
    RtInt const *inputAOV;
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

    // If we don't have a display match, then just pass on through.
    // This needs to be "success" so that the values carry through.
    if (!data->displayCount) {
        return 0;
    }
    
    // return 0;
    
    // Write to the AOVs.
    RixDisplayServices *displayServices = sctx->GetDisplayServices();
    for (int i = 0; i < NUM_INPUTS; i++) {
        DEBUG("HERE 4.%d", i);
        if (data->displays[i] < 0 || !inputExists[i]) {
            continue;
        }
        DEBUG("HERE 4.%d.2: %d", i, data->displays[i]);
        // continue;
        for (int p = 0; p < sctx->numPts; p++) {
            // NOTE: We are using Write instead of Splat, which does not have any
            // filtering or anything.
            displayServices->Write(data->displays[i], sctx->integratorCtxIndex[p], inputColor[i][p]);
        }
        DEBUG("HERE 4.%d.3", i);
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

