#include <stdio.h>
#include <XPLMDefs.h>
#include "MainFrame.h"
#include <string>


/*
* XPluginStart
*
* Our start routine registers our window and does any other initialization we
* must do.
*
*/
PLUGIN_API int XPluginStart(
    char *		outName,
    char *		outSig,
    char *		outDesc)
{
    strcpy(outName, "XFFSTracker");
    strcpy(outSig, "ffsimulateur2.fr");
    strcpy(outDesc, "Multijoueur FFS2.");
    MainFrame* window = MainFrame::Create();
    return 1;
}

/*
* XPluginStop
*
* Our cleanup routine deallocates our window.
*
*/
PLUGIN_API void	XPluginStop(void)
{
    MainFrame::Kill();
}

/*
* XPluginDisable
*
* We do not need to do anything when we are disabled, but we must provide the handler.
*
*/
PLUGIN_API void XPluginDisable(void)
{
}

/*
* XPluginEnable.
*
* We don't do any enable-specific initialization, but we must return 1 to indicate
* that we may be enabled at this time.
*
*/
PLUGIN_API int XPluginEnable(void)
{
    return 1;
}

/*
* XPluginReceiveMessage
*
* We don't have to do anything in our receive message handler, but we must provide one.
*
*/
PLUGIN_API void XPluginReceiveMessage(
    XPLMPluginID	inFromWho,
    long			inMessage,
    void *			inParam)
{
}
