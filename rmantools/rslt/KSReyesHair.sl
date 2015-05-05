#ifndef KS_REYES_PLASTIC_H
#define KS_REYES_PLASTIC_H
/*
<rman id="rslt">
slim 1 extensions pixar_db {
extensions pixar {} {
    
    template void KSReyesHair {

        userdata {
            rfm_nodeid 2000022
            rfm_classification \
                shader/surface:rendernode/RenderMan/shader/surface:swatch/rmanSwatch
        }

        codegenhints {
            shaderobject {

                initDiffuse {
                    f:prelighting
                    diffuseRootColor
                    diffuseTipColor
                    diffuseGain
                    diffuseReflectionGain
                    diffuseTransmitGain
                    nDiffuseSamples
                }

                initSpecular {
                    f:prelighting
                    specularColor
                    specularShift
                    specularWidth
                    specularTransmitGain
                    specularReflectionGain
    
                    nSpecularSamples
                }

                lighting {
                    f:initDiffuse
                    f:initSpecular
                    WriteGPAOVs
                }

            }
        }
    
        collection void Diffuse {

            parameter color diffuseRootColor {
                default {1 1 1}
            }

            parameter color diffuseTipColor {
                default {1 1 1}
            }

        }

        collection void Specular {

            parameter color specularColor {
                default {1 1 1}
            }
            
            parameter float specularShift {
                detail cantvary
                subtype slider 
                range {5 10 0.1}
                default 7.5
            }

            parameter float specularWidth {
                detail cantvary
                subtype slider 
                range {5 10 0.1}
                default 7.5
            }

        }

        # parameter float ior {
        #     label "IOR"
        #     subtype slider
        #     range {1 2.5 .01}
        #     default 1.5 
        # }
        # parameter float mediaIor {
        #     label "Media IOR"
        #     detail cantvary
        #     subtype slider 
        #     range {1 2.5 .01}
        #     default 1 
        # }

        collection void Gains {
    
            parameter float diffuseGain {
                detail cantvary
                subtype slider 
                range {0 1 0.01}
                default 1
            }

            parameter float diffuseReflectionGain {
                detail cantvary
                subtype slider 
                range {0 1 0.01}
                default 1
            }

            parameter float diffuseTransmitGain {
                detail cantvary
                subtype slider 
                range {0 1 0.01}
                default 1
            }

            parameter float specularReflectionGain {
                detail cantvary
                subtype slider 
                range {0 1 0.01}
                default 0.04
            }

            parameter float specularTransmitGain {
                detail cantvary
                subtype slider 
                range {0 1 0.01}
                default 0.5
            }

        }
    
        collection void Details {

            parameter float nDiffuseSamples {
                detail cantvary
                subtype slider 
                range {0 1024 16}
                default 256
            }

            parameter float nSpecularSamples {
                detail cantvary
                subtype slider 
                range {0 64 1}
                default 16
            }

            parameter float WriteGPAOVs {
                detail cantvary
                subtype switch
                default 0
            }


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
#include <stdrsl/Hair.h>

RSLINJECT_preamble

RSLINJECT_shaderdef
{

    RSLINJECT_members


    // Signal that we don't do anything special with opacity.
    uniform float __computesOpacity = 0;

    stdrsl_ShadingContext m_shadingCtx;
    stdrsl_Fresnel m_fresnel;
    stdrsl_Hair m_hair;

    uniform string m_lightGroups[];
    uniform float m_nLightGroups;

    varying color diffuseColor;

    public void construct() {
        m_shadingCtx->construct();
        option("user:lightgroups",  m_lightGroups);
        m_nLightGroups = arraylength(m_lightGroups);
    }

    public void begin() {
        RSLINJECT_begin

        m_shadingCtx->initHair(0); // 0 -> trust the normals.

        normal surfaceN = N;

        // Get the scalp surface normal
        if (readprimvar("surface_normal", surfaceN)) { // maya fur
            m_shadingCtx->m_Nn = normalize(surfaceN);
        } else if (readprimvar("N_srf", surfaceN)) { // shave
            m_shadingCtx->m_Nn = normalize(surfaceN); 
        } else if (readprimvar("n_surf", surfaceN)) { // yeti
            m_shadingCtx->m_Nn = normalize(surfaceN);
        }

        // Reinit (excluding bitangent).
        vector bitangent = m_shadingCtx->m_Bitangent;
        m_shadingCtx->reinit();
        m_shadingCtx->m_Bitangent = bitangent;

        // m_fresnel->init(m_shadingCtx, mediaIor, ior);

    }

    public void initDiffuse() {
        RSLINJECT_initDiffuse

        // We don't apply this until much later.
        diffuseColor = mix(diffuseRootColor, diffuseTipColor, v);

        m_hair->initDiffuse(m_shadingCtx,
            diffuseGain, // diffuse gain
            diffuseReflectionGain, // diffuse reflection gain
            diffuseTransmitGain, // diffuse transmit gain
            color(1), // root color
            color(1)  // tip color
        );
    }

    public void initSpecular() {
        RSLINJECT_initSpecular
        m_hair->initSpecular(m_shadingCtx, nSpecularSamples,
            color(specularTransmitGain), // color(m_fresnel->m_Kt), // transmit color
            specularShift, // shift highlight from root to tip [5, 10]
            specularWidth, // highlight width [5, 10]
            specularReflectionGain, // iorRefl ??
            -1 // index (for picking directions)
        );
    }

    void writeAOVs(string pattern; color diffuseDirect, specularDirect,
        unshadowedDiffuseDirect, unshadowedSpecularDirect, diffuseIndirect
    ) {

        writeaov(format(pattern, "Diffuse"), diffuseColor * (diffuseDirect + diffuseIndirect)); // Same as GP.
        writeaov(format(pattern, "Specular"), specularColor * specularDirect); // DIRECT ONLY! Same as GP.

        writeaov(format(pattern, "DiffuseDirect"), diffuseDirect);
        writeaov(format(pattern, "SpecularDirect"), specularDirect);
        writeaov(format(pattern, "DiffuseDirectNoShadow"), unshadowedDiffuseDirect);
        writeaov(format(pattern, "SpecularDirectNoShadow"), unshadowedSpecularDirect);

        // We find these shadows make a bit more sense.
        writeaov(format(pattern, "DiffuseShadowMult"), diffuseDirect / unshadowedDiffuseDirect);
        writeaov(format(pattern, "SpecularShadowMult"), specularDirect / unshadowedSpecularDirect);

        if (WriteGPAOVs) {
            writeaov(format(pattern, "DiffuseShadow" ), diffuseColor  * (unshadowedDiffuseDirect  - diffuseDirect )); // Same as GP.
            writeaov(format(pattern, "SpecularShadow"), specularColor * (unshadowedSpecularDirect - specularDirect)); // Same as GP.
        }

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
        color groupedDiffuseDirect[];
        color groupedSpecularDirect[];
        color groupedUnshadowedDiffuseDirect[];
        color groupedUnshadowedSpecularDirect[];

        if (depth == 0 && m_nLightGroups != 0) {
            // We only need all of this data when we are writing AOVs.
            directlighting(this, lights,
                "diffuseresult", diffuseDirect,
                "specularresult", specularDirect,
                "unshadoweddiffuseresult", unshadowedDiffuseDirect,
                "unshadowedspecularresult", unshadowedSpecularDirect,

                "lightgroups", m_lightGroups,
                "groupeddiffuseresults", groupedDiffuseDirect,
                "groupedspecularresults", groupedSpecularDirect,
                "groupedunshadoweddiffuseresults", groupedUnshadowedDiffuseDirect,
                "groupedunshadowedspecularresults", groupedUnshadowedSpecularDirect,

                "integrationdomain", "sphere", 
                "mis", 1
            );
        } else {
            directlighting(this, lights,
                "diffuseresult", diffuseDirect,
                "specularresult", specularDirect,
                "unshadoweddiffuseresult", unshadowedDiffuseDirect,
                "unshadowedspecularresult", unshadowedSpecularDirect,
                
                "integrationdomain", "sphere", 
                "mis", 1
            );
        }


        color diffuseIndirect = indirectdiffuse(P, normalize(N), nDiffuseSamples);
        color specularIndirect = indirectspecular(this);

        Ci += diffuseColor  * (diffuseDirect  + diffuseIndirect ) \
            + specularColor * (specularDirect + specularIndirect);

        if (depth == 0) {

            writeAOVs("%s",
                diffuseDirect, specularDirect,
                unshadowedDiffuseDirect, unshadowedSpecularDirect,
                diffuseIndirect
            );

            writeaov("DiffuseColor", diffuseColor); // Not written by GP.
            writeaov("DiffuseIndirect", diffuseIndirect); // Not written by GP.
            writeaov("SpecularIndirect", specularIndirect); // Same as GP.

            uniform float i;
            for (i = 0; i < m_nLightGroups; i += 1) {
                writeAOVs(concat("Grouped%s_", m_lightGroups[i]),
                    groupedDiffuseDirect[i],
                    groupedSpecularDirect[i],
                    groupedUnshadowedDiffuseDirect[i],
                    groupedUnshadowedSpecularDirect[i],
                    color(0)
                );
            }

        }

    }


    public void evaluateSamples(string distribution; output __radiancesample samples[]) {
        if (distribution == "diffuse" && nDiffuseSamples > 0) {
            m_hair->evalDiffuseSamps(m_shadingCtx, samples);
        }
        if (distribution != "diffuse" && nSpecularSamples > 0) {
            m_hair->evalSpecularSamps(m_shadingCtx, samples);
        }
    }

    public void generateSamples(string distribution; output __radiancesample samples[]) {
        if (distribution != "diffuse" && nSpecularSamples > 0) {
            m_hair->genSpecularSamps(m_shadingCtx, samples);
        }
    }
}

#endif


