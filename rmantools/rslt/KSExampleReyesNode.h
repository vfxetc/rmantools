#ifndef KS_EXAMPLE_REYES_NODE_H
#define KS_EXAMPLE_REYES_NODE_H
/*
<rman id="rslt">
slim 1 extensions pixar_db {
extensions pixar {} {

    template void KSExampleReyesNode {
        
        description {
            Example of a RSLT node; computes color noise for a given location.
        }

        # Classifications include:
        #     - rendernode/RenderMan/texture/3d: RfM makes a place3dTexture
        #       and connects (at least) the "pm" attribute. See more in
        #       $RMSTREE/lib/rfm/rsl/rfmTexture3d.rslt
        #     - rendernode/RenderMan/texture/2d: Likely very similar to 3d above.
        #     - rendernode/RenderMan/utility: Does nothing special AFAICT.
        #     - swatch/rmanSwatch: Use RenderMan to create the swatch (maybe?)
        userdata {
            rfm_nodeid 2000002
            rfm_classification rendernode/RenderMan/utility
        }

        parameter point location {
            description { The location to sample noise. }
        }
        
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




void KSExampleReyesNode(
    point location;
    output color resultColor;
) {
    resultColor = noise(location);
}



#endif
