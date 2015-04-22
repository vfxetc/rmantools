#ifndef KS_REYES_NOISE_H
#define KS_REYES_NOISE_H
/* 
<rman id="rslt">
slim 1 extensions pixar_db {
extensions pixar rfm_ {

    template void KSExampleReyesNode {

        userdata {
            rfm_nodeid 2000012
            rfm_classification \
                shader/surface:rendernode/RenderMan/shader/surface:swatch/rmanSwatch
        }

        parameter point location
        parameter { output color } resultColor { detail mustvary }

        RSLInclude KSExampleReyesNode.h
        RSLFunction {
            // No extra source for KSExampleReyesNode.
        }

    }
}
}
</rman>
*/




/* The "rfm_" prefix corresponds to "extensions pixar rfm_" in the slim template. */
void rfm_KSExampleReyesNode(point location; output color resultColor;)
{
    resultColor = noise(location);
}



#endif
