#ifndef KS_REYES_NOISE_H
#define KS_REYES_NOISE_H
/* 
<rman id="rslt">
slim 1 extensions pixar_db {
extensions pixar {} {

    template void KSReyesSides {
        
        description {
            Side mask that can use geometric normals.
        }

        userdata {
            rfm_nodeid 2000015
            rfm_classification rendernode/RenderMan/utility
        }


        parameter color frontColor {
            default {1 1 1}
        }

        parameter color backColor {
            default {0 0 0}
        }

        parameter float useGeometricNormal {
            detail cantvary
            subtype switch
            description {
                Use geometric normal rather than shading normal?
            }
            default 0
        }
        
        parameter { output float } resultAlpha { detail mustvary }

        parameter { output color } resultColor { detail mustvary }

        RSLInclude KSReyesSides.h
        RSLFunction {
            // No extra source for KSReyesSides.
        }

    }
}
}
</rman>
*/




void KSReyesSides(
    color frontColor;
    color backColor;
    float useGeometricNormal;
    output float resultAlpha;
    output color resultColor;
) {

    normal NN;
    if (useGeometricNormal > 0) { 
        NN = Ng;
    } else {
        NN = N;
    }

    float NdotI = NN.I;
    resultAlpha = step(0, NdotI);
    resultColor = mix(frontColor, backColor, resultAlpha);
}



#endif
