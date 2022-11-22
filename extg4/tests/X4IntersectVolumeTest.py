#!/usr/bin/env python
"""
X4IntersectVolumeTest.py : 2D scatter plots of geometry intersect positions
============================================================================

* typically used from xxv.sh 

"""

import os, logging, numpy as np
from opticks.ana.fold import Fold
from opticks.ana.pvplt import mpplt_add_contiguous_line_segments 

log = logging.getLogger(__name__)
np.set_printoptions(suppress=True, edgeitems=5, linewidth=200,precision=3)

try:
    import matplotlib.pyplot as mp 
except ImportError:
    mp = None
pass

try:
    import pyvista as pv
    from pyvista.plotting.colors import hexcolors  
    themes = "default dark paraview document".split()
    pv.set_plot_theme(themes[1])
except ImportError:
    pv = None
    hexcolors = None
pass

#mp = None
pv = None

X,Y,Z = 0,1,2

efloatlist_ = lambda ekey:list(map(float, filter(None, os.environ.get(ekey,"").split(","))))

if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    log.info("X4IntersectVolumeTest.py")

    reldir = os.environ.get("CXS_RELDIR", "extg4/X4IntersectVolumeTest" ) 
    geom = os.environ.get("X4IntersectVolumeTest_GEOM", "body_phys")

    colors = "red green blue cyan magenta yellow pink orange purple lightgreen".split()
    gcol = "grey"

    basedir = os.path.expandvars(os.path.join("/tmp/$USER/opticks",reldir, geom ))
    transforms = np.load(os.path.join(basedir, "transforms.npy"))
    transforms_meta = np.loadtxt( os.path.join(basedir, "transforms_meta.txt"), dtype=np.object ) 
    soname_prefix = os.path.commonprefix(list(map(str, transforms_meta))) 

    figsdir = os.path.join(basedir, "figs")
    if not os.path.isdir(figsdir):
        os.makedirs(figsdir)
    pass

    savefig = True
    figname = "isect"

    log.info("figsdir %s " % figsdir)
    log.info("basedir %s" % basedir)
    log.info("transforms.shape %s" % str(transforms.shape))
    log.info("transforms_meta %s" % ",".join(transforms_meta))
    log.info("soname_prefix %s" % soname_prefix)

    topline = "X4IntersectVolumeTest.py"
    botline = "%s/%s " % (reldir, geom)
    thirdline = soname_prefix

    isects = {}
    for soname in transforms_meta:
        isects[soname] = Fold.Load(basedir, soname, "X4Intersect")
    pass
    LABELS = list(filter(None,os.environ.get("LABELS","").split(",")))
    SZ = float(os.environ.get("SZ",3))


    EXTRA = os.environ.get("EXTRA", "/tmp/U4PMTFastSimTest/pid726.npy")
    if os.path.exists(EXTRA):
        extra = np.load(EXTRA)
        print("loaded extra %s from EXTRA %s " % (str(extra.shape), EXTRA))
    else:
        extra = None
    pass     


     
    REVERSE = int(os.environ.get("REVERSE","0")) == 1
    print("REVERSE : %d " % REVERSE)

    size = np.array([1280, 720])
    #H,V = Z,X
    H,V = X,Z

    if mp: 
        fig, ax = mp.subplots(figsize=size/100.) # 100 dpi 
        ax.set_aspect('equal')
       
        soname0 = transforms_meta[0]
        isect0 = isects[soname0]
        gpos = isect0.gs[:,5,:3]    # last line of the transform is translation

        hlim = gpos[:,H].min(), gpos[:,H].max()
        vlim = gpos[:,V].min(), gpos[:,V].max()
        ax.set_xlim( hlim )
        ax.set_ylim( vlim )

        if not "NOGS" in os.environ:
            ax.scatter( gpos[:,H], gpos[:,V], s=SZ, color=gcol ) 
        pass
        num = len(transforms_meta)
        for j in range(num):
            i = num - 1 - j  if REVERSE else j              
            soname = transforms_meta[i] 
            isect = isects[soname]
            tran = np.float32(transforms[i])
            ipos = isect.isect[:,0,:3] + tran[3,:3]
            color = colors[ i % len(colors)]

            label = str(soname)[len(soname_prefix):]
            if label[0] == "_": label = label[1:]   # seems labels starting "_" have special meaning to mpl, causing problems
            label = label.replace("solid","s")

            select = len(LABELS) == 0 or label in LABELS
            print(" %2d : %30s : %15s : %s " % (i, soname, label, select ))
            if select:
                ax.scatter( ipos[:,H], ipos[:,V], s=SZ, color=color, label=label ) 
            pass
        pass

        if not extra is None:
            print("extra scatter EXTRA %s " % EXTRA)
            ModelTriggerYES = extra[np.where(extra[:,3,0].astype(np.int64) == 1)]  
            ModelTriggerNO = extra[np.where(extra[:,3,0].astype(np.int64) == 0)]  

            if "ModelTriggerYES" in os.environ:
                ax.scatter( ModelTriggerYES[:,0,H], ModelTriggerYES[:,0,V], s=50, color="red"   )
                thirdline += " ModelTriggerYES" 
            pass
            if "ModelTriggerNO" in os.environ:
                ax.scatter( ModelTriggerNO[:,0,H], ModelTriggerNO[:,0,V], s=50, color="blue"  )
                thirdline += " ModelTriggerNO" 
            pass

            mpplt_add_contiguous_line_segments(ax, extra[:,0,:3], axes=(H,V), label="extra" )
        pass

        #loc = "lower left"
        loc = "upper right"
        LOC = os.environ.get("LOC",loc)
        if LOC != "skip":
            ax.legend(loc=LOC,  markerscale=4)
        pass
        fig.suptitle("\n".join([topline,botline,thirdline]))
        fig.show()

        if savefig:
            figpath = os.path.join(figsdir,figname+"_mpplt.png")
            log.info("saving figpath %s " % figpath)
            fig.savefig(figpath)
        pass 
    pass


