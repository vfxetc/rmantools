#ifndef KS_REYES_PLASTIC_H
#define KS_REYES_PLASTIC_H
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

                begin {
                    inputAOV
                }

                displacement {
                    bumpAmount
                    bumpScale
                }

                prelighting {
                    ior
                    mediaIor
                }

                initDiffuse {
                    f:prelighting
                    diffuseSamples
                }

                initSpecular {
                    f:prelighting
                    specularRoughness
                    specularAnisotropy
                    specularSamples
                }

                lighting {
                    f:initDiffuse
                    f:initSpecular
                    diffuseColor
                    specularColor
                    writeGPAOVs
                }

            }
        }
    
        collection void Basics {

            parameter color diffuseColor {
                default {1 1 1}
            }

            parameter color specularColor {
                default {1 1 1}
            }

            parameter float specularRoughness {
                default 0.001
            }

            parameter float specularAnisotropy {
                subtype slider
                range {-1 1 .0001}
                default 0
            } 

            parameter float ior {
                label "IOR"
                description {
                    Index of Refraction of the surface.
                }
                subtype slider
                range {1 2.5 .01}
                default 1.5 
            }

            parameter float mediaIor {
                label "Media IOR"
                description {
                    Index of Refraction of the media that the surface is in (e.g. the air).
                }
                subtype slider 
                range {1 2.5 .01}
                default 1 
            }

        }

        collection void Bump {

            parameter float bumpAmount {
                default 0
            }

            parameter float bumpScale {
                detail cantvary
                subtype slider 
                range {0 1 0.001}
                default 1
            }

        }

        collection void Details {
        
            parameter float diffuseSamples {
                detail cantvary
                subtype slider 
                range {1 1024 1}
                default 256
            }

            parameter float specularSamples {
                detail cantvary
                subtype slider 
                range {1 64 1}
                default 16
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


RSLINJECT_preamble

RSLINJECT_shaderdef
{

    RSLINJECT_members


    // Signal that we don't do anything special with opacity.
    uniform float __computesOpacity = 0;

    stdrsl_ShadingContext m_shadingCtx;
    stdrsl_Fresnel m_fresnel;
    stdrsl_Lambert m_diffuse;
    stdrsl_SpecularAS m_specular;

    uniform string m_lightGroups[];
    uniform float m_nLightGroups;


    public void construct() {
        m_shadingCtx->construct();
        option("user:lightgroups",  m_lightGroups);
        m_nLightGroups = arraylength(m_lightGroups);
    }

    public void begin() {
        RSLINJECT_begin
        m_shadingCtx->init();
    }

    public void displacement(output point P; output normal N)
    {
        RSLINJECT_displacement
        if (bumpAmount != 0 && bumpScale != 0) {
            m_shadingCtx->displace(m_shadingCtx->m_Ns, bumpAmount * bumpScale, "bump");
            m_shadingCtx->reinit();
        }
    }

    public void prelighting(output color Ci, Oi) {
        RSLINJECT_prelighting
        m_fresnel->init(m_shadingCtx, mediaIor, ior);
    }

    public void initDiffuse() {
        RSLINJECT_initDiffuse
        m_diffuse->init(m_shadingCtx, color(m_fresnel->m_Kt), 1, diffuseSamples);
    }

    public void initSpecular() {
        RSLINJECT_initSpecular
        m_specular->init(m_shadingCtx,
            color(m_fresnel->m_Kr), // The fresnel is not automatically used by the spec.
            specularRoughness,
            specularAnisotropy,
            1, // Roughness scale.
            1, // Minimum samples.
            specularSamples // Maximum samples.
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
                "groupedunshadowedspecularresults", groupedUnshadowedSpecularDirect
            );
        } else {
            directlighting(this, lights,
                "diffuseresult", diffuseDirect,
                "specularresult", specularDirect,
                "unshadoweddiffuseresult", unshadowedDiffuseDirect,
                "unshadowedspecularresult", unshadowedSpecularDirect
            );
        }


        color diffuseIndirect = indirectdiffuse(P, normalize(N), diffuseSamples);
        color specularIndirect = indirectspecular(this);

        Ci = diffuseColor  * (diffuseDirect  + diffuseIndirect ) \
           + specularColor * (specularDirect + specularIndirect);

        Ci *= Oi;

        if (depth == 0) {

            writeAOVs("%s",
                diffuseDirect, specularDirect,
                unshadowedDiffuseDirect, unshadowedSpecularDirect,
                diffuseIndirect
            );

            writeaov("DiffuseColor", diffuseColor); // Not written by GP.
            writeaov("SpecularColor", specularColor); // Not written by GP.
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
            m_diffuse->evalDiffuseSamps(m_shadingCtx, m_fresnel, samples);
        }
        if (distribution != "diffuse" && specularSamples > 0) {
            m_specular->evalSpecularSamps(m_shadingCtx, m_fresnel, samples);
        }
    }

    public void generateSamples(string distribution; output __radiancesample samples[]) {
        if (distribution != "diffuse" && specularSamples > 0) {
            m_specular->genSpecularSamps(m_shadingCtx, m_fresnel, distribution, samples);
        }
    }
}

#endif


