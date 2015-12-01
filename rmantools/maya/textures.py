from __future__ import absolute_import

import os
from concurrent.futures import ThreadPoolExecutor

from maya import cmds

from ..textures import txmake


_texture_attrs = (
    ('file', 'fileTextureName'),
    ('RMSGeoAreaLight', 'lightcolormap'),
    ('PxrBump', 'filename'),
    ('PxrTexture', 'filename'),
)


def txmake_all():

    executor = ThreadPoolExecutor(4)
    futures = []

    for node_type, attr in _texture_attrs:
        for node in cmds.ls(type=node_type) or ():
            src = cmds.getAttr(node + '.' + attr).strip()
            if not src:
                continue

            if src.endswith('.tex'):
                dst = src
                src = dst.rsplit('.', 1)[0]
            else:
                dst = src + '.tex'

            if not os.path.exists(src):
                print 'MISSING TEXTURE from %s.%s %s: %s' % (node_type, attr, node, src)
                continue

            if os.path.exists(dst) and os.path.getmtime(src) <= os.path.getmtime(dst):
                print 'Skipping up-to-date %s.%s %s: %s' % (node_type, attr, node, dst)
                future = None
            else:
                print 'Txmaking %s.%s %s: %s' % (node_type, attr, node, src)
                future = executor.submit(txmake, src, dst, newer=False)
            futures.append((future, node, attr, dst))

    for future, node, attr, dst in futures:
        if future:
            future.result() # Wait for it
        cmds.setAttr(node + '.' + attr, dst, type='string')
