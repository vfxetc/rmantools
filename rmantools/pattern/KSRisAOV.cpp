#include "RixPattern.h"
#include "RixShadingUtils.h"
#include "RixColorUtils.h"
#include "RixIntegrator.h"
#include <cstring>
#include <stdint.h>
#include <stdio.h>


#define DEBUG(x, ...) // printf("[KSRisAOV] " x "\n", ##__VA_ARGS__); fflush(stdout);


class KSRisAOV : public RixPattern
{
public:

    // Stubs.
    KSRisAOV() {}
    virtual ~KSRisAOV() {}
    virtual int Init(RixContext &, char const *pluginpath) { return 0; }
    virtual void Finalize(RixContext &) {}
    
    virtual RixSCParamInfo const *GetParamTable();
    
    virtual int CreateInstanceData(RixContext &ctx,
                                   char const *handle,
                                   RixParameterList const *plist,
                                   InstanceData *idata);
    
    virtual int ComputeOutputParams(RixShadingContext const *,
                                    RtInt *n, RixPattern::OutputSpec **outputs,
                                    RtConstPointer instanceData,
                                    RixSCParamInfo const *);

};



// NOTE: This MUST match the order of parameters returned from GetParamTable().
enum paramId
{
    // Passthroughs / triggers.
    k_inputAOV,
    k_resultAOV,
    k_inputPassColor,
    k_resultPassColor,
    k_inputPassFloat,
    k_resultPassFloat,

    // Results for first float and color (mostly for b/c).
    k_resultFloat,
    k_resultColor,
    
    // Offsets for dynamic creation.
    k_nameOffset,
    k_inputOffset,
};


RixSCParamInfo const *
KSRisAOV::GetParamTable()
{
    static RixSCParamInfo s_ptable[] =
    {
        // Passthroughs / triggers.
        RixSCParamInfo("inputAOV", k_RixSCInteger),
        RixSCParamInfo("resultAOV", k_RixSCInteger, k_RixSCOutput),
        RixSCParamInfo("inputPassFloat", k_RixSCFloat),
        RixSCParamInfo("resultPassFloat", k_RixSCFloat, k_RixSCOutput),
        
        RixSCParamInfo("inputPassColor", k_RixSCColor),
        RixSCParamInfo("resultPassColor", k_RixSCColor, k_RixSCOutput),
        RixSCParamInfo("resultFloat", k_RixSCFloat, k_RixSCOutput),
        RixSCParamInfo("resultColor", k_RixSCColor, k_RixSCOutput),
        
        // The names are a little awkward here for maintain b/c.
        #define PARAM_INFO(index, lower, upper) \
            RixSCParamInfo(#lower  "Name" #index , k_RixSCString), \
            RixSCParamInfo("input" #upper #index , k_RixSC##upper),
        PARAM_INFO(, float, Float) // Single float channel.
        
        #define COLOR_PARAM_INFO(index) PARAM_INFO(index, color, Color)
        COLOR_PARAM_INFO() // Original color channel.
        COLOR_PARAM_INFO(1)
        COLOR_PARAM_INFO(2)
        COLOR_PARAM_INFO(3)
        COLOR_PARAM_INFO(4)
        COLOR_PARAM_INFO(5)
        COLOR_PARAM_INFO(6)
        COLOR_PARAM_INFO(7)
        COLOR_PARAM_INFO(8)
        COLOR_PARAM_INFO(9)

        RixSCParamInfo(), // End of table.
    };
    return &s_ptable[0];
}

#define NUM_MAPPINGS 11
struct ChannelMapping {
    char const *name; // Mainly for debugging.
    int nameParam;
    int inputParam;
    int resultParam;
    int type;
} mappings[] = {
    {"float" , k_nameOffset    , k_inputOffset    , k_resultFloat, k_RixSCFloat},
    {"color0", k_nameOffset + 2, k_inputOffset + 2, k_resultColor, k_RixSCColor},
    #define COLOR_MAPPING(i) \
        {"color" #i, k_nameOffset + 2 * (i + 1), k_inputOffset + 2 * (i + 1), -1, k_RixSCColor}
    COLOR_MAPPING(1),
    COLOR_MAPPING(2),
    COLOR_MAPPING(3),
    COLOR_MAPPING(4),
    COLOR_MAPPING(5),
    COLOR_MAPPING(6),
    COLOR_MAPPING(7),
    COLOR_MAPPING(8),
    COLOR_MAPPING(9)
};


struct MyData {
public:
    int displayCount;
    int displays[NUM_MAPPINGS];
};

int
KSRisAOV::CreateInstanceData(RixContext &ctx,
                               char const *handle,
                               RixParameterList const *plist,
                               InstanceData *idata)
{

    RtInt dataSize = sizeof(MyData);
    MyData *data = (MyData*)malloc(dataSize);
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
    
    data->displayCount = 0;

    // Match them up to what was requested.
    for (int i = 0; i < NUM_MAPPINGS; i++) {
        
        // Signal that we haven't found it.
        data->displays[i] = -1;
        
        // Grab the requested AOV name.
        RtConstString name = NULL;
        plist->EvalParam(mappings[i].nameParam, 0, &name);
        if (!name || !strlen(name)) {
            continue;
        }
        
        DEBUG("%s = \"%s\"", mappings[i].name, name);
        
        RixSCType shadingType;
        RixSCConnectionInfo connInfo;
        plist->GetParamInfo(mappings[i].inputParam, &shadingType, &connInfo);
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
KSRisAOV::ComputeOutputParams(RixShadingContext const *sctx,
                                RtInt *noutputs, OutputSpec **outputs,
                                RtConstPointer instanceData,
                                RixSCParamInfo const *ignored)
{
    // Only execute on camera hits
    if (!(sctx->scTraits.primaryHit && sctx->scTraits.eyePath)) {
        return 1;
    }
    
    MyData const *data = (MyData*)instanceData;
    
    RixShadingContext::Allocator pool(sctx);
    OutputSpec* out = pool.AllocForPattern<OutputSpec>(5); // I'm lazy.
    *outputs = out;
    *noutputs = 0;
    #define DONE_OUTPUT { out++; (*noutputs)++; }
    
    RtColorRGB defaultColor; // Just something to pull from without complaining.
    RtFloat defaultFloat;
    RtColorRGB const *inputColor;
    RtFloat const *inputFloat;
    RixSCDetail detail;
    
    // Pull on the inputAOV plug to make others evaluate.
    RtInt const *inputAOV = NULL;
    sctx->EvalParam(k_inputAOV, -1, &inputAOV);
    // ...and push it back out.
    out->paramId = k_resultAOV;
    out->detail = k_RixSCUniform;
    out->value = (RtPointer)pool.AllocForPattern<RtInt>(1);
    ((RtInt*)out->value)[0] = inputAOV ? inputAOV[0] : 0;
    DONE_OUTPUT
    
    // Passthrough other trigger values.
    detail = sctx->EvalParam(k_inputPassFloat, -1, &inputFloat, &defaultFloat);
    if (detail) {
        out->paramId = k_resultPassFloat;
        out->detail = detail;
        out->value = inputFloat;
        DONE_OUTPUT
    }
    detail = sctx->EvalParam(k_inputPassColor, -1, &inputColor, &defaultColor);
    if (detail) {
        out->paramId = k_resultPassColor;
        out->detail = detail;
        out->value = inputColor;
        DONE_OUTPUT
    }
    
    // Write to the AOVs.
    RixDisplayServices *displayServices = sctx->GetDisplayServices();
    for (int i = 0; i < NUM_MAPPINGS; i++) {
        
        if (data->displays[i] < 0) {
            continue;
        }
        
        // Read the input.
        if (mappings[i].type == k_RixSCFloat) {
            detail = sctx->EvalParam(mappings[i].inputParam, -1, &inputFloat, &defaultFloat);
        } else {
            detail = sctx->EvalParam(mappings[i].inputParam, -1, &inputColor, &defaultColor);
        }
        if (!detail) {
            // We likely don't need to check this due to check in instance data.
            continue;
        }
        
        // Pass it back out if requested.
        if (mappings[i].resultParam >= 0) {
            // DEBUG("writing through %s", mappings[i].name);
            out->paramId = mappings[i].resultParam;
            out->detail = detail;
            out->value = mappings[i].type == k_RixSCFloat ? (void*)inputFloat : (void*)inputColor;
            DONE_OUTPUT
        }
        
        for (int p = 0; p < sctx->numPts; p++) {
            // NOTE: We are using Write instead of Splat, which does not have any
            // filtering or anything.
            if (mappings[i].type == k_RixSCFloat) {
                displayServices->Write(data->displays[i], sctx->integratorCtxIndex[p], inputFloat[detail == k_RixSCVarying ? p : 0]);
            } else {
                displayServices->Write(data->displays[i], sctx->integratorCtxIndex[p], inputColor[detail == k_RixSCVarying ? p : 0]);
            }
        }
    }
    
    return 0;
}


RIX_PATTERNCREATE
{
    return new KSRisAOV();
}

RIX_PATTERNDESTROY
{
    delete ((KSRisAOV*)pattern);
}

