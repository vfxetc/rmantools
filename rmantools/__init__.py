import os


def get_envvars():

    rmantools = os.path.dirname(os.path.abspath(__file__))

    return {

        # Pull in the config.
        'RMS_SCRIPT_PATHS': rmantools,

        # Individual paths.
        'RMAN_RIX_PATH': os.path.join(rmantools, 'pattern'),
        'RMAN_SHADER_PATH': os.path.join(rmantools, 'pattern'),
        # 'RMAN_DISPLAY_PATH': os.path.join(rmantools, 'display')
        # 'RMAN_FILTER_PATH': os.path.join(rmantools, 'filter')
        # 'RMAN_PROCEDURAL_PATH': os.path.join(rmantools, 'procedural')

        'XBMLANGPATH': os.path.join(rmantools, 'icons'),

    }


