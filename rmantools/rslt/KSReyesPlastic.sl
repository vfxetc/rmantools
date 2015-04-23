#ifndef KS_REYES_DIFFUSE_CLASS_H
#define KS_REYES_DIFFUSE_CLASS_H
/*
<rman id="rslt">
slim 1 extensions pixar_db {
extensions pixar {} {
    
    template void KSReyesPlastic {

        userdata {
            rfm_nodeid 2000021
            rfm_classification \
                shader/surface:rendernode/RenderMan/shader/surface:swatch/rmanSwatch
        }

        codegenhints {
            shaderobject {
                surface {
                    diffuseColor
                    ambientColor
                    specularColor
                    roughness
                }
            }
        }
    
        parameter color diffuseColor {
            provider parameterlist
            default {1 1 1}
        }

        parameter color ambientColor {
            provider parameterlist
            default {1 1 1}
        }

        parameter color specularColor {
            provider parameterlist
            default {1 1 1}
        }

        parameter float roughness {
            provider parameterlist
            default 0.1
        }

        RSLSource ShaderPipeline _thisfile_

    }
}}
</rman>
*/

RSLINJECT_preamble

RSLINJECT_shaderdef
{

    RSLINJECT_members

    public void surface(output color Ci; output color Oi;)
    {
        RSLINJECT_surface

        uniform float depth = 0;
        rayinfo("depth", depth);

        normal Nn = normalize(N);
        normal Nf = faceforward(Nn, I);
        vector V  = -normalize(I);

        Ci = Cs;

        color ambientResult = ambientColor * ambient();
        color specularResult = specularColor * specular(Nf, V, roughness);

        color diffuseIrradiance = 0;
        illuminance(P, Nf, PI/2) {
            vector Ln = normalize(L);
            diffuseIrradiance += Cl * Ln.Nf;
            string group;
            float gotGroup = getvar(light, "__group", group);
            if (gotGroup) printf("group: %f %s\n", gotGroup, group);
        }
        color diffuseResult = diffuseColor * diffuseIrradiance;

        Ci *= ambientResult + diffuseResult;
        Ci += specularResult;

        Oi = Os;
        Ci *= Oi;

        if (depth == 0) {
            writeaov("Diffuse", diffuseResult);
            writeaov("Ambient", ambientResult);
            writeaov("Specular", specularResult);
        }

    }

}

#endif


