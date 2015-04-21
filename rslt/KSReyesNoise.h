#ifndef KS_REYES_NOISE_H
#define KS_REYES_NOISE_H
/* 
<rman id="rslt">
slim 1 extensions pixar_db {
extensions pixar {} {

    template void KSReyesNoise {

        userdata {
            rfm_nodeid 2000012
            rfm_classification \
                shader/surface:rendernode/RenderMan/shader/surface:swatch/rmanSwatch
        }

        parameter point Q
        parameter { output color } outC { detail mustvary }

        RSLInclude KSReyesNoise.h
        RSLFunction { // No extra source for KSReyesNoise. }

    }
}
}
</rman>
*/

void KSReyesNoise(point Q; output color outC;)
{
    outC = noise(Q);
}

#endif

