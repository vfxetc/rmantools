
rmantools_env="$(python -c 'from rmantools import get_envvars; print "\n".join("export %s=%s" % x for x in get_envvars().iteritems())')"

if [[ $? == 0 ]]; then
    eval "$rmantools_env"
fi
