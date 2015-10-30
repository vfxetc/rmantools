#ifndef KS_REYES_HAIR_FADE_H
#define KS_REYES_HAIR_FADE_H
/* 
<rman id="rslt">
slim 1 extensions pixar_db {
extensions pixar {} {

    template void KSReyesHairFade {
        
        description {
            Side mask that can use geometric normals.
        }

        userdata {
            rfm_nodeid 2000016
            rfm_classification rendernode/RenderMan/utility
        }

        parameter color inputColor {
            default {1 0 1}
        }

        parameter color fadeColor {
            default {1 1 1}
        }

        parameter float rootFade {
            subtype slider
            range {0 1 .0001}
            default 0
        }

        parameter float tipFade {
            subtype slider
            range {0 1 .0001}
            default 1
        }

        parameter float bias {
            subtype slider
            range {0 4 .0001}
            default 1
        }

        parameter float assumeIsShave {
            detail cantvary
            subtype switch
            description {
                Use geometric normal rather than shading normal?
            }
            default 0
        }

        parameter { output color } resultColor { detail mustvary }

        RSLInclude KSReyesHairFade.h
        RSLFunction {
            // No extra source for KSReyesHairFade.
        }

    }
}
}
</rman>
*/




void KSReyesHairFade(
    color inputColor;
    color fadeColor;
    float rootFade;
    float tipFade;
    float bias;
    float assumeIsShave;
    output color resultColor;
) {
    float shaveAmbDiff; // Just for the signal.
    float isShave = assumeIsShave != 0 ? 1 : readprimvar("SHAVEambdiff", shaveAmbDiff);
    if (isShave != 0) {
        float blend = pow(v, bias);
        float fade = mix(rootFade, tipFade, blend);
        resultColor = mix(inputColor, fadeColor, fade);
    } else {
        resultColor = inputColor;
    }
}



#endif
