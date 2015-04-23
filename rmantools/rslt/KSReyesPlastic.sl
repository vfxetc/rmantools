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
                initDiffuse {
                    diffuseColor
                    ambientColor
                }
                lighting {
                    f:initDiffuse
                }
                diffuselighting {
                    f:initDiffuse
                }
                specularlighting {
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

#include <stdrsl/Colors.h>
#include <stdrsl/Fresnel.h>
#include <stdrsl/Lambert.h>
#include <stdrsl/Math.h>
#include <stdrsl/OrenNayar.h>
#include <stdrsl/RadianceSample.h>
#include <stdrsl/ShadingContext.h>
#include <stdrsl/SpecularAS.h>


RSLINJECT_preamble

RSLINJECT_shaderdef
{

    RSLINJECT_members


    uniform float specularRoughness = .008;
    uniform float __computesOpacity = 0;

    uniform float m_nSamplesSpec = 36;
    uniform float m_nSamplesDiffuse = 256;

    uniform float m_ior = 1.8;
    uniform float m_mediaIor = 1;

    stdrsl_ShadingContext m_shadingCtx;
    stdrsl_Fresnel m_fresnel;
    stdrsl_Lambert m_diffuse;
    stdrsl_SpecularAS m_specular;


    public void construct() {
        m_shadingCtx->construct();
    }

    public void begin() {
        m_shadingCtx->init();
    }

    public void prelighting(output color Ci, Oi) {
    }

    public void initDiffuse() {
        RSLINJECT_initDiffuse
        m_diffuse->init(m_shadingCtx, color(1), 1, m_nSamplesDiffuse);
    }

    public void initSpecular() {
        RSLINJECT_initSpecular
        m_fresnel->init(m_shadingCtx, m_mediaIor, m_ior);
        m_specular->init(m_shadingCtx,
            color(1), // We will do the mult later.
            specularRoughness,
            0, // Anistrophy ratio.
            1, // Roughness scale.
            1, // Minimum samples.
            m_nSamplesSpec // Maximum samples.
        );
    }

    public void lighting(output color Ci, Oi)
    {
        RSLINJECT_lighting
        initDiffuse();
        initSpecular();

        float depth = 0;
        rayinfo("depth", depth);

        shader lights[] = getlights();

        color diffuseDirect = 0;
        color specularDirect = 0;
        color unshadowedDiffuseDirect = 0;
        color unshadowedSpecularDirect = 0;
        directlighting(this, lights,
            "diffuseresult", diffuseDirect,
            "specularresult", specularDirect,
            "unshadoweddiffuseresult", unshadowedDiffuseDirect,
            "unshadowedspecularresult", unshadowedSpecularDirect

            /*
            "groupeddiffuseresult", groupedDiffuseDirect,
            "groupedspecularresult", groupedSpecularDirect,
            "groupedunshadoweddiffuseresult", groupedUnshadowedDiffuseDirect,
            "groupedunshadowedspecularresult", groupedUnshadowedSpecularDirect
            */

        );

        color diffuseIndirect = indirectdiffuse(P, normalize(N), m_nSamplesDiffuse);
        color diffuseLighting = diffuseDirect + diffuseIndirect;

        color specularIndirect = indirectspecular(this);
        color specularLighting = specularDirect + specularIndirect;

        color diffuseOutput = diffuseColor * diffuseLighting;
        color specularOutput = specularColor * specularLighting;

        Ci += diffuseOutput + specularOutput;

        if (depth == 0) {
            writeaov("DiffuseDirect", diffuseDirect);
            writeaov("SpecularDirect", specularDirect);
            writeaov("DiffuseDirectNoShadow", unshadowedDiffuseDirect);
            writeaov("SpecularDirectNoShadow", unshadowedSpecularDirect);
            // writeaov("DiffuseShadow", diffuseColor * (unshadowedDiffuseDirect - diffuseDirect)); // Same as GP.
            writeaov("DiffuseShadowMult", diffuseDirect / unshadowedDiffuseDirect);
            writeaov("SpecularShadowMult", specularDirect / unshadowedSpecularDirect);
            // writeaov("SpecularShadow", specularColor * (unshadowedSpecularDirect - specularDirect)); // Same as GP.
            writeaov("DiffuseColor", diffuseColor);
            writeaov("DiffuseIndirect", diffuseIndirect); // Not written by GP.
            writeaov("SpecularIndirect", specularIndirect); // Same as GP.
            writeaov("Diffuse", diffuseOutput); // Same as GP.
            writeaov("Specular", specularColor * specularDirect); // DIRECT ONLY! Same as GP.
        }

    }

    public void evaluateSamples(string distribution; output __radiancesample samples[]) {
        if(distribution == "diffuse") {
            m_diffuse->evalDiffuseSamps(m_shadingCtx, m_fresnel, samples);
        } else if (distribution == "specular") {
            m_specular->evalSpecularSamps(m_shadingCtx, m_fresnel, samples);
        }
    }

    public void generateSamples(string distribution; output __radiancesample samples[]) {
        if (distribution != "diffuse" && m_nSamplesSpec > 0) {
            m_specular->genSpecularSamps(m_shadingCtx, m_fresnel, distribution, samples);
        }
    }
}

#endif


