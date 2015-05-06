#ifndef KS_REYES_HAIR_H
#define KS_REYES_HAIR_H
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

                begin {
                    inputAOV
                }

                opacity {
                    rootOpacity
                    tipOpacity
                }

                initDiffuse {
                    f:prelighting

                    diffuseRootColor
                    diffuseTipColor
                    diffuseGain
                    diffuseReflectGain
                    diffuseTransmitGain
                    diffuseSamples
                }

                initSpecular {
                    f:prelighting

                    specularRootColor
                    specularTipColor
                    specularShift
                    specularWidth
                    specularTransmitGain
                    specularReflectGain
                    specularSamples
                }

                lighting {
                    f:initDiffuse
                    f:initSpecular
                    lightingSamples
                    writeGPAOVs
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

            parameter color specularRootColor {
                default {1 1 1}
            }

            parameter color specularTipColor {
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

        collection void Opacity {

            parameter color rootOpacity {
                default {1 1 1}
            }

            parameter color tipOpacity {
                default {1 1 1}
            }

            parameter float __computesOpacity {
                label "Compute Opacity"
                description {
                    If the shader doesn't compute opacity, the renderer can
                    take some shortcuts.
                }
                detail cantvary
                subtype switch
                default 0
            }

        }

        collection void Gains {
    
            parameter float diffuseGain {
                detail cantvary
                subtype slider 
                range {0 1 0.01}
                default 0.5
            }

            parameter float diffuseReflectGain {
                detail cantvary
                subtype slider 
                range {0 1 0.01}
                default 0.5
            }

            parameter float diffuseTransmitGain {
                detail cantvary
                subtype slider 
                range {0 1 0.01}
                default 0.5
            }

            parameter float specularReflectGain {
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

            parameter float diffuseSamples {
                detail cantvary
                subtype slider 
                range {0 128 1}
                default 16
            }

            parameter float specularSamples {
                detail cantvary
                subtype slider 
                range {0 64 1}
                default 4
            }

            parameter float lightingSamples {
                description {
                    Override the number of direct lighting samples;
                    hair doesn't usually need the normal amount.
                }
                detail cantvary
                subtype slider 
                range {0 64 1}
                default 8
            }

            parameter float writeGPAOVs {
                label "Write GP AOVs"
                description {
                    Write all AOVs to be compatible with GP shaders.
                }
                detail cantvary
                subtype switch
                default 0
            }

            parameter float inputAOV {
                description {
                    This exists only to trigger others nodes to evaluate, and 
                    is not used in any way.
                }
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


    stdrsl_ShadingContext m_shadingCtx;
    stdrsl_Hair m_hair;

    uniform string m_lightGroups[];
    uniform float m_nLightGroups;

    varying color diffuseColor; // Mocking the naming convention of parameters.
    varying color specularColor; // Mocking the naming convention of parameters.

    public void construct() {
        m_shadingCtx->construct();
        option("user:lightgroups",  m_lightGroups);
        m_nLightGroups = arraylength(m_lightGroups);
    }

    public void begin() {

        RSLINJECT_begin

        m_shadingCtx->initHair(0); // 0 -> We don't trust the normals.

        // Fetch the "scalp" surface normal (for various packages).
        normal surfaceN = N;
        if (readprimvar("surface_normal", surfaceN)) { // Maya fur
            m_shadingCtx->m_Nn = normalize(surfaceN);
        } else if (readprimvar("N_srf", surfaceN)) { // Shave
            m_shadingCtx->m_Nn = normalize(surfaceN); 
        } else if (readprimvar("n_surf", surfaceN)) { // Yeti
            m_shadingCtx->m_Nn = normalize(surfaceN);
        }

        // Reinit (excluding bitangent).
        vector bitangent = m_shadingCtx->m_Bitangent;
        m_shadingCtx->reinit();
        m_shadingCtx->m_Bitangent = bitangent;

    }


    public void opacity(output color Oi) {

        RSLINJECT_opacity

        if(__computesOpacity != 0) {
            Oi = mix(rootOpacity, tipOpacity, v);
        } else { 
            // This shouldn't get called, but...
            Oi = Os;
        }
    }

    public void initDiffuse() {
        RSLINJECT_initDiffuse

        // We don't apply this until much later.
        diffuseColor = mix(diffuseRootColor, diffuseTipColor, v);

        m_hair->initDiffuse(m_shadingCtx,
            diffuseGain,
            diffuseReflectGain,
            diffuseTransmitGain,
            color(1), // root color
            color(1)  // tip color
        );
    }

    public void initSpecular() {
        RSLINJECT_initSpecular

        // We don't apply this until much later.
        specularColor = mix(specularRootColor, specularTipColor, v);

        m_hair->initSpecular(m_shadingCtx, specularSamples,
            color(specularTransmitGain), // transmitColor
            specularShift,
            specularWidth,
            specularReflectGain,
            -1 // index for picking directions; -1 -> random
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

        if (writeGPAOVs) {
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

        uniform float _lightingSamples = (m_shadingCtx->m_SampleMgr->m_PathTracing) == 1 ? 1 : lightingSamples;

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
                "mis", 1,
                "arealightsamples", _lightingSamples
            );
        } else {
            directlighting(this, lights,
                "diffuseresult", diffuseDirect,
                "specularresult", specularDirect,
                "unshadoweddiffuseresult", unshadowedDiffuseDirect,
                "unshadowedspecularresult", unshadowedSpecularDirect,

                "integrationdomain", "sphere", 
                "mis", 1,
                "arealightsamples", _lightingSamples
            );
        }


        color diffuseIndirect = indirectdiffuse(P, normalize(N), diffuseSamples);
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
        if (distribution == "diffuse" && diffuseSamples > 0) {
            m_hair->evalDiffuseSamps(m_shadingCtx, samples);
        }
        if (distribution != "diffuse" && specularSamples > 0) {
            m_hair->evalSpecularSamps(m_shadingCtx, samples);
        }
    }

    public void generateSamples(string distribution; output __radiancesample samples[]) {
        if (distribution != "diffuse" && specularSamples > 0) {
            m_hair->genSpecularSamps(m_shadingCtx, samples);
        }
    }
}

#endif


