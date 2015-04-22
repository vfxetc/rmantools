rmantools
=========

A collection of Python, SL, OSL, and C++ for RenderMan 19.


Bootstrapping
-------------

This project [will be] a Python module which contains RenderMan tools. It
registers a series of entrypoints to initialize the pre/post-launch environment.

A `make` step [will be added] to `python setup.py build` to build the compiled
tools.

This is rather fragile, and I apologize.



Patterns (RIS)
--------------

- KSRisAOV: Write an arbitrary colour or float to a given AOV.
- KSRisSides: Front and back colour switch.


Code Stubs
----------

- `pattern/KSStubExprPattern.args`: Example of an SeEXPR pattern (for RIS).
- `pattern/KSStubOslPattern.{osl,args}`: Example of an OSL pattern (for RIS).
- `rslt/KSExampleRSLTNode.h`: Example of an RSLT shader node (for REYES) using RSL.
- `rslt/KSExampleRSLTRoot.sl` Example of an RSLT shader root node (for REYES) using RSL 2.0.

We are missing a non-2.0 RSL root shader, which we think is possible but didn't
find a good example of with RenderMan.
