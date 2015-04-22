#ifndef KS_REYES_NOISE_H
#define KS_REYES_NOISE_H
/* 
<rman id="rslt">
slim 1 extensions pixar_db {
extensions pixar rfm_ {

    template void KSExampleReyesNode {
        
        
        # Classifications include:
        #     - rendernode/RenderMan/texture/3d: RfM connects a new
        #       place3dTexture to the "pm" attribute.
        #     - rendernode/RenderMan/texture/2d: RfM likely does something,
        #       but I haven't tried yet.
        #     - rendernode/RenderMan/utility: does nothing special.
        userdata {
            rfm_nodeid 2000012
            rfm_classification rendernode/RenderMan/utility
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
