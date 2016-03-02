#include "RixPattern.h"
#include "RixShadingUtils.h"
#include "RixColorUtils.h"
#include "RixIntegrator.h"
#include <cstring>
#include <stdint.h>
#include <stdio.h>


#define DEBUG(x, ...) printf("[KSRisHasPrim] " x "\n", ##__VA_ARGS__); fflush(stdout);


class KSRisHasPrim : public RixPattern
{
public:

    // Stubs.
    KSRisHasPrim() {}
    virtual ~KSRisHasPrim() {}
    virtual int Init(RixContext &, char const *pluginpath) { return 0; }
    virtual void Finalize(RixContext &) {}
    
    virtual RixSCParamInfo const *GetParamTable();
    
    virtual int ComputeOutputParams(RixShadingContext const *,
                                    RtInt *n, RixPattern::OutputSpec **outputs,
                                    RtConstPointer instanceData,
                                    RixSCParamInfo const *);

};



// NOTE: This MUST match the order of parameters returned from GetParamTable().
enum paramId
{
    k_result,
    k_primvarName,
    k_assumeTrue
};


RixSCParamInfo const *
KSRisHasPrim::GetParamTable()
{
    static RixSCParamInfo s_ptable[] =
    {
        RixSCParamInfo("result", k_RixSCFloat, k_RixSCOutput),
        RixSCParamInfo("primvarName", k_RixSCString),
        RixSCParamInfo("assumeTrue", k_RixSCFloat),
        RixSCParamInfo(), // End of table.
    };
    return &s_ptable[0];
}


int
KSRisHasPrim::ComputeOutputParams(RixShadingContext const *sctx,
                                RtInt *noutputs, OutputSpec **outputs,
                                RtConstPointer instanceData,
                                RixSCParamInfo const *ignored)
{
    
    RixShadingContext::Allocator pool(sctx);
    OutputSpec* out = pool.AllocForPattern<OutputSpec>(1);
    *outputs = out;
    *noutputs = 1;

    out->paramId = k_result;
    out->detail = k_RixSCUniform;
    float *resultValue = (float*)pool.AllocForPattern<RtFloat>(1);
    out->value = resultValue;
    
    // Grab the default variable.
    // NOTE: This takes the first value if it is varying.
    RtFloat const *assumeTrue;
    if (sctx->EvalParam(k_assumeTrue, -1, &assumeTrue, NULL)) {
        *resultValue = assumeTrue[0];
    }
    
    // Grab the requested primvar name.
    RtConstString *primvarName;
    sctx->EvalParam(k_primvarName, -1, &primvarName, NULL);
    if (!*primvarName || !strlen(*primvarName)) {
        return 0; // Not set; bail!
    }
        
    // DEBUG("primvarName = \"%s\"", *primvarName);
    
    // Check for presense.
    RixSCType type;
    RtInt arrayLen;
    RixSCDetail detail = sctx->GetPrimVar(*primvarName, &type, &arrayLen);
    if (detail) {
        *resultValue = 1.f;
    }
    
    return 0;
}


RIX_PATTERNCREATE
{
    return new KSRisHasPrim();
}

RIX_PATTERNDESTROY
{
    delete ((KSRisHasPrim*)pattern);
}

