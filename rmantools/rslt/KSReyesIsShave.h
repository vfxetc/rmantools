#ifndef KS_REYES_IS_SHAVE_H
#define KS_REYES_IS_SHAVE_H
/*
<rman id="rslt">
slim 1 extensions pixar_db {
extensions pixar {} {

    template void KSReyesIsShave {
        
        description {
            Determine if the current surface is hair from Shave and a Haircut.
        }

        userdata {
            rfm_nodeid 2000017
            rfm_classification rendernode/RenderMan/utility
        }
        
        parameter string primvarName {
            detail cantvary
            label "Primitive Variable"
            description {
                Which float primvar to test for; defaults to "SHAVEambdiff".
            }
            default "SHAVEambdiff"
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

        RSLInclude KSReyesIsShave.h
        RSLFunction {
            // No extra source for KSReyesIsShave.
        }

    }
}
}
</rman>
*/



void KSReyesIsShave(
    string primvarName;
    float assumeTrue;
    output float result;
) {
    float primvarValue; // Just for the signal.
    result = assumeTrue != 0 ? 1 : readprimvar(primvarName, primvarValue);
}



#endif
