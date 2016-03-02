#ifndef KS_REYES_HAS_PRIMVAR_H
#define KS_REYES_HAS_PRIMVAR_H
/*
<rman id="rslt">
slim 1 extensions pixar_db {
extensions pixar {} {

    template void KSReyesHasPrimvar {
        
        description {
            Determine if the current surface is hair from Shave and a Haircut.
        }

        userdata {
            rfm_nodeid 2000018
            rfm_classification rendernode/RenderMan/utility
        }
        
        parameter string primvarName {
            detail cantvary
            label "Primitive Variable"
            description {
                Which float primvar to test for.
            }
            default ""
        }
        
        parameter float assumeTrue {
            detail cantvary
            subtype switch
            description {
                Output true.
            }
            default 0
        }

        parameter { output float } result { detail mustvary }

        RSLInclude KSReyesHasPrimvar.h
        RSLFunction {
            // No extra source for KSReyesHasPrimvar.
        }

    }
}
}
</rman>
*/



void KSReyesHasPrimvar(
    string primvarName;
    float assumeTrue;
    output float result;
) {
    float primvarValue; // Just for the signal.
    result = assumeTrue != 0 ? 1 : readprimvar(primvarName, primvarValue);
}



#endif
