import subprocess


def txmake(src, dst=None, wrap='periodic', newer=True, resize='up', resize_square=True):

    if dst is None:
        dst = src + '.tex'

    if wrap not in ('periodic', 'clamp', 'black'):
        raise ValueError('bad wrap mode', wrap)
    if resize not in ('up', 'down', 'round', 'none'):
        raise ValueError('bad resize more', resize)

    if resize != 'none' and resize_square:
        resize += '-'

    args = ['txmake', '-mode', wrap, '-resize', resize]
    if newer:
        args.append('-newer')
    args.extend((src, dst))

    subprocess.check_call(args)
    return dst

