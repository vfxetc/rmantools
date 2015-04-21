#ifndef KS_REYES_DIFFUSE_H
#define KS_REYES_DIFFUSE_H
/*
<rman id="rslt">
slim 1 extensions pixar_db {
extensions pixar {} {
    
    template void KSReyesDiffuse {

        userdata {
            rfm_nodeid 2000010
            rfm_classification \
                shader/surface:rendernode/RenderMan/shader/surface:swatch/rmanSwatch
        }

        codegenhints {
            codegenclass Surface_rfm
            globals {RFM_SURFACE_SETUP();}
            pregen {RFM_SURFACE_BEGIN();}
            postgen {RFM_SURFACE_END();}
        }

        RSLFunction { // No extra source for KSReyesNoise. }

    }
}}
</rman>
*/

RSLINJECT_preamble

RSLINJECT_shaderdef
{

    // member variables produced by code generator
    RSLINJECT_members

    public void light(output vector Ci; output color Oi;)
    {
        normal Nn = normalize(N);
        Ci = Cs * diffuse(Nn);
        Oi = Os;
    }

}

#endif