#ifndef KS_REYES_HAIRMAPPING_H
#define KS_REYES_HAIRMAPPING_H
/*
<rman id="rslt">
slim 1 extensions pixar_db {
extensions pixar {} {

    template void KSReyesHairMapping {

        description {
            returns the uv coordinate on a hair follicle.
        }

        # Classifications include:
        #     - rendernode/RenderMan/texture/3d: RfM makes a place3dTexture
        #       and connects (at least) the "pm" attribute. See more in
        #       $RMSTREE/lib/rfm/rsl/rfmTexture3d.rslt
        #     - rendernode/RenderMan/texture/2d: Likely very similar to 3d above.
        #     - rendernode/RenderMan/utility: Does nothing special AFAICT.
        #     - swatch/rmanSwatch: Use RenderMan to create the swatch (maybe?)
        userdata {
            rfm_nodeid 2000013
            rfm_classification rendernode/RenderMan/utility
        }
        parameter { output float } uCoord { detail mustvary }
        parameter { output float } vCoord { detail mustvary }

        RSLInclude KSReyesHairMapping.h
        RSLFunction {
            // No extra source for KSReyesHairMapping.
        }

    }
}
}
</rman>
*/

void KSReyesHairMapping(output float uCoord; output float vCoord;)
{
	uCoord = u;
	vCoord = v;
}

#endif
