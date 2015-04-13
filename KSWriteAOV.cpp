#include "RixPattern.h"
#include "RixShadingUtils.h"
#include "RixColorUtils.h"
#include "RixIntegrator.h"
#include <cstring>
#include <stdint.h>
#include <stdio.h>


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
    k_resultColor = 0,
    k_resultFloat,
    k_resultPassColor,
    k_resultPassFloat,
    k_resultAOV,

    k_inputColor,
    k_colorName,
    k_inputFloat,
    k_floatName,
    k_inputPassColor,
    k_inputPassFloat,
    k_inputAOV,
};


RixSCParamInfo const *
KSWriteAOV::GetParamTable()
{
    static RixSCParamInfo s_ptable[] =
    {
        RixSCParamInfo("resultColor"    , k_RixSCColor, k_RixSCOutput),
        RixSCParamInfo("resultFloat"    , k_RixSCFloat, k_RixSCOutput),
        RixSCParamInfo("resultPassColor", k_RixSCColor, k_RixSCOutput),
        RixSCParamInfo("resultPassFloat", k_RixSCFloat, k_RixSCOutput),
        RixSCParamInfo("resultAOV"      , k_RixSCInteger, k_RixSCOutput),

        RixSCParamInfo("inputColor"    , k_RixSCColor),
        RixSCParamInfo("colorName"     , k_RixSCString),
        RixSCParamInfo("inputFloat"    , k_RixSCFloat),
        RixSCParamInfo("floatName"     , k_RixSCString),
        RixSCParamInfo("inputPassColor", k_RixSCColor),
        RixSCParamInfo("inputPassFloat", k_RixSCFloat),
        RixSCParamInfo("inputAOV"      , k_RixSCInteger),

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

    RtConstString *colorName, *floatName;

    // Get the names.
    sctx->EvalParam(k_colorName, -1, &colorName, NULL, false);
    if (colorName && !strlen(*colorName)) {
        colorName = NULL;
    }
    sctx->EvalParam(k_floatName, -1, &floatName, NULL, false);
    if (floatName && !strlen(*floatName)) {
        floatName = NULL;
    }

    // Get the displays.
    // TODO: Get this into CreateInstanceData.
    RixRenderState *renderState;
    renderState = (RixRenderState *)sctx->GetRixInterface(k_RixRenderState);
    RixRenderState::FrameInfo frameInfo;
    renderState->GetFrameInfo(&frameInfo);
    RixIntegratorEnvironment *integratorEnv;
    integratorEnv = (RixIntegratorEnvironment*)frameInfo.integratorEnv;
    RtInt numDisplays = integratorEnv->numDisplays;
    RixDisplayChannel const* displayChannels = integratorEnv->displays;

    // Get inputs.
    RtColorRGB defaultColor; // Just something for it pull from without complaining.
    RtFloat defaultFloat;
    RtColorRGB const *inputColor, *inputPassColor;
    RtFloat const *inputFloat, *inputPassFloat;
    bool hasInputColor     = sctx->EvalParam(k_inputColor, -1, &inputColor, &defaultColor, true);
    bool hasInputPassColor = sctx->EvalParam(k_inputPassColor, -1, &inputPassColor, &defaultColor, true);
    bool hasInputFloat     = sctx->EvalParam(k_inputFloat, -1, &inputFloat, &defaultFloat, true);
    bool hasInputPassFloat = sctx->EvalParam(k_inputPassFloat, -1, &inputPassFloat, &defaultFloat, true);

    // Pull on the inputAOV plug to make others evaluate.
    RtInt const *inputAOV;
    sctx->EvalParam(k_inputAOV, -1, &inputAOV);

    // Don't bother writing AOVs if we don't have inputs.
    colorName = hasInputColor ? colorName : NULL;
    floatName = hasInputFloat ? floatName : NULL;

    // Figure out which displays we are writing to.
    RtInt colorDisplay = -1;
    RtInt floatDisplay = -1;
    if (colorName || floatName) {
        for (unsigned int i = 0; i < numDisplays; i++) {
            if (colorName && !strcmp(displayChannels[i].channel, *colorName)) {
                colorDisplay = displayChannels[i].id;
                if (!floatName || floatDisplay >= 0) {
                    break;
                }
            }
            if (floatName && !strcmp(displayChannels[i].channel, *floatName)) {
                floatDisplay = displayChannels[i].id;
                if (!colorName || colorDisplay >= 0) {
                    break;
                }
            }
        }
    }

    RixShadingContext::Allocator pool(sctx);

    OutputSpec* out = pool.AllocForPattern<OutputSpec>(5); // I'm lazy.
    *outputs = out;
    *noutputs = 1 + hasInputColor + hasInputPassColor + hasInputFloat + hasInputPassFloat;

    // printf("color:%d (%s) float:%d (%s) outputs:%d\n", colorDisplay, colorName ? *colorName : "", floatDisplay, floatName ? *floatName : "", *noutputs);

    if (hasInputColor) {
        out->paramId = k_resultColor;
        out->detail = k_RixSCVarying;
        out->value = (RtPointer)inputColor; // Pass through the results.
        out++;
    }
    if (hasInputPassColor) {
        out->paramId = k_resultPassColor;
        out->detail = k_RixSCVarying;
        out->value = (RtPointer)inputPassColor; // Pass through the results.
        out++;
    }
    if (hasInputFloat) {
        out->paramId = k_resultFloat;
        out->detail = k_RixSCVarying;
        out->value = (RtPointer)inputFloat; // Pass through the results.
        out++;
    }
    if (hasInputPassFloat) {
        out->paramId = k_resultPassFloat;
        out->detail = k_RixSCVarying;
        out->value = (RtPointer)inputPassFloat; // Pass through the results.
        out++;
    }

    out->paramId = k_resultAOV;
    out->detail = k_RixSCUniform;
    out->value = (RtPointer)pool.AllocForPattern<RtInt>(1);

    // If we don't have a display match, then just pass on through.
    // This needs to be "success" so that the values carry through.
    if (colorDisplay < 0 && floatDisplay < 0) {
        return 0;
    }

    // Write to the AOVs.
    RixDisplayServices *displayServices = sctx->GetDisplayServices();
    for (int i = 0; i < sctx->numPts; i++) {
        // NOTE: We are using Write instead of Splat, which does not have any
        // filtering or anything.
        if (colorDisplay >= 0) {
            displayServices->Write(colorDisplay, sctx->integratorCtxIndex[i], inputColor[i]);
        }
        if (floatDisplay >= 0) {
            displayServices->Write(floatDisplay, sctx->integratorCtxIndex[i], inputFloat[i]);
        }
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

