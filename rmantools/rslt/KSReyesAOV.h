#ifndef KS_REYES_AOV_H
#define KS_REYES_AOV_H
/* 
<rman id="rslt">
slim 1 extensions pixar_db {
extensions pixar {} {

    template void KSReyesAOV {
        
        description {
            Write many AOVs at once.
        }

        userdata {
            rfm_nodeid 2000014
            rfm_classification rendernode/RenderMan/utility
        }
        
        foreach i {1 2 3 4 5 6 7 8 9 10} {
            parameter string name$i  { detail cantvary }
            parameter color  color$i { }
        }

        parameter float premultiply {
            description {
                Premultiply all values (using Oi by default).
            }
            detail cantvary
            subtype switch
            default 1
        }

        parameter float useCustomPremultiply {
            description {
                Use the supplied value to premultuply.
            }
            detail cantvary
            subtype switch
            default 0
        }

        parameter color customPremultiply { default {1 1 1} }

    
        parameter float inputAOV {
            detail mustvary
            description {
                This exists only to trigger others nodes to evaluate, and 
                is not used in any way.
            }
            default 0
        }

        parameter { output float } outputAOV {
            detail mustvary
            description {
                Just to be tugged on by other nodes.
            }
        }

        RSLInclude KSReyesAOV.h
        RSLFunction {
            // No extra source for KSReyesAOV.
        }

    }
}
}
</rman>
*/



void KSReyesAOV(

    #define PARAMS(i) uniform string name##i; varying color color##i;
    PARAMS(1)
    PARAMS(2)
    PARAMS(3)
    PARAMS(4)
    PARAMS(5)
    PARAMS(6)
    PARAMS(7)
    PARAMS(8)
    PARAMS(9)
    PARAMS(10)

    uniform float premultiply;
    uniform float useCustomPremultiply;
    varying color customPremultiply;

    float inputAOV;
    output float outputAOV;

) {

    extern color Oi;

    varying color premultiplyValue = color(1);
    if (premultiply != 0) {
        premultiplyValue = (useCustomPremultiply != 0) ? customPremultiply : Oi;
    }

    #define WRITEAOV(i) \
        if (name##i != "") { \
            writeaov(name##i, color##i * premultiplyValue); \
        }
    WRITEAOV(1)
    WRITEAOV(2)
    WRITEAOV(3)
    WRITEAOV(4)
    WRITEAOV(5)
    WRITEAOV(6)
    WRITEAOV(7)
    WRITEAOV(8)
    WRITEAOV(9)
    WRITEAOV(10)

    // outputAOV = inputAOV; // Why not actually do it?

}



#endif
