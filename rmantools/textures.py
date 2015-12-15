from subprocess import Popen, PIPE


class TxmakeError(EnvironmentError):
    pass


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

    proc = Popen(args, stdin=PIPE, stdout=PIPE, stderr=PIPE)
    out, err = proc.communicate()

    out = out.strip()
    if out:
        print out

    err = err.strip()
    if proc.returncode:
        raise TxmakeError(proc.returncode, err)
    elif err:
        print err
    
    return dst

