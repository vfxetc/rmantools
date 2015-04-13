#include "RixPattern.h"
#include "RixShadingUtils.h"
#include "RixColorUtils.h"
#include "RixIntegrator.h"
#include <cstring>
#include <stdint.h>


class KSWriteAOV : public RixPattern
{
public:

    KSWriteAOV();
    virtual ~KSWriteAOV();

    virtual int Init(RixContext &, char const *pluginpath);
    virtual RixSCParamInfo const *GetParamTable();
    virtual void Finalize(RixContext &);
    virtual int CreateInstanceData(RixContext &ctx,
                                   char const *handle,
                                   RixParameterList const *plist,
                                   InstanceData *idata);
    virtual int ComputeOutputParams(RixShadingContext const *,
                                    RtInt *n, RixPattern::OutputSpec **outputs,
                                    RtConstPointer instanceData,
                                    RixSCParamInfo const *);

};


KSWriteAOV::KSWriteAOV()
{
}


KSWriteAOV::~KSWriteAOV()
{
}


int
KSWriteAOV::Init(RixContext &ctx, char const *pluginpath)
{
    return 0;
}


enum paramId
{
    k_colorResult=0,
    k_colorValue,
    k_aovName
};


RixSCParamInfo const *
KSWriteAOV::GetParamTable()
{
    static RixSCParamInfo s_ptable[] =
    {
        RixSCParamInfo("colorResult", k_RixSCColor, k_RixSCOutput),
        RixSCParamInfo("colorValue" , k_RixSCColor),
        RixSCParamInfo("aovName"    , k_RixSCString),
        RixSCParamInfo(), // end of table
    };
    return &s_ptable[0];
}


void
KSWriteAOV::Finalize(RixContext &ctx)
{
}



int
KSWriteAOV::CreateInstanceData(RixContext &ctx,
                               char const *handle,
                               RixParameterList const *plist,
                               InstanceData *idata)
{
    return 0;
}

int
KSWriteAOV::ComputeOutputParams(RixShadingContext const *sctx,
                                RtInt *noutputs, OutputSpec **outputs,
                                RtConstPointer instanceData,
                                RixSCParamInfo const *ignored)
{
    // Only execute on camera hits
    if (!(sctx->scTraits.primaryHit && sctx->scTraits.eyePath)) {
        return 1;
    }

    // Get our aovName
    RtConstString *aovName;
    sctx->EvalParam(k_aovName, -1, &aovName, NULL, false);
    if (!strlen(*aovName)) {
        return 1;
    }

    // Get integrator environment.
    RixRenderState *renderState;
    renderState = (RixRenderState *)sctx->GetRixInterface(k_RixRenderState);
    RixRenderState::FrameInfo frameInfo;
    renderState->GetFrameInfo(&frameInfo);
    RixIntegratorEnvironment *integratorEnv;
    integratorEnv = (RixIntegratorEnvironment*)frameInfo.integratorEnv;

    // Get displays
    RtInt numDisplays = integratorEnv->numDisplays;
    RixDisplayChannel const* displayChannels = integratorEnv->displays;

    RtInt displayId = -1;
    for (unsigned int i = 0; i < numDisplays; i++) {
        if (!strcmp(displayChannels[i].channel, *aovName)) {
            displayId = displayChannels[i].id;
            break;
        }
    }
    if (displayId < 0) {
        return 1;
    }


    RtColorRGB const *colorValue;
    sctx->EvalParam(k_colorValue, -1, &colorValue, NULL, true);

    RixShadingContext::Allocator pool(sctx);

    // Allocate and bind our output
    OutputSpec* out = pool.AllocForPattern<OutputSpec>(1);
    *outputs = out;
    *noutputs = 1;

    RtColorRGB* colorResult = pool.AllocForPattern<RtColorRGB>(sctx->numPts);
    out[0].paramId = k_colorResult;
    out[0].detail = k_RixSCVarying;
    out[0].value = (RtPointer)colorResult;

    //  Get the display services to write out the AOV.
    RixDisplayServices *dispSvc = sctx->GetDisplayServices();

    for (int i = 0; i < sctx->numPts; i++) {
        colorResult[i] = colorValue[i];
        dispSvc->Write(displayId,
                       sctx->integratorCtxIndex[i],
                       colorValue[i]);
    }
    
    return 0;
}


RIX_PATTERNCREATE
{
    return new KSWriteAOV();
}

RIX_PATTERNDESTROY
{
    delete ((KSWriteAOV*)pattern);
}

