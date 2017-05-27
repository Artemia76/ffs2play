TEMPLATE = subdirs
SUBDIRS = ffs2core ffs2play

ffs2core.subdir = src/ffs2core
ffs2play.subdir = src/ffs2play

ffs2play.depends = ffs2core
