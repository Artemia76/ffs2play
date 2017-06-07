TEMPLATE = subdirs
SUBDIRS = \
    ffs2core \
    ffs2play \
    xffs2play

ffs2play.depends = ffs2core
xff2play.depends = ffs2core