#ifndef KS_REYES_NOISE_H
#define KS_REYES_NOISE_H
/* 
<rman id="rslt">
slim 1 extensions pixar_db {
extensions pixar {} {

    template void KSExampleCoshaderNode {
        
        description {
            xxx
        }

        userdata {
            rfm_nodeid 2000003
            rfm_classification rendernode/RenderMan/utility
        }

        parameter { output color } surfaceResult { detail mustvary }

        parameter {shader[]} coshadersToSum {
            provider parameterlist
            detail cantvary
            subtype connection
        }

        RSLInclude KSExampleCoshaderNode.h
        RSLFunction {
            // No extra source for KSExampleCoshaderNode.
        }

    }
}
}
</rman>
*/


void KSExampleCoshaderNode(
    output color surfaceResult;
    uniform shader coshaders[];
) {
    surfaceResult = color(0);
    uniform float i;
    uniform float len = arraylength(coshaders);
    for (i = 0; i < len; i += 1) {
        color outColor, outOpacity;
        coshaders[i]->surface(outColor, outOpacity);
        surfaceResult += outColor;
    }
}


#endif
