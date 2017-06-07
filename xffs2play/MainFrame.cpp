#include "MainFrame.h"

MainFrame* MainFrame::m_ptMainFrame = NULL;

MainFrame* MainFrame::Create()
{
	if (m_ptMainFrame == NULL)
	{
		m_ptMainFrame = new MainFrame();
	}
	return m_ptMainFrame;
}

void MainFrame::Kill()
{
	if (m_ptMainFrame != NULL)
		delete m_ptMainFrame;
}

MainFrame::MainFrame() : QObject (NULL)
{
	gWindow = XPLMCreateWindow(
		50, 600, 300, 200,
		1,
		&MainFrame::DrawWindowCallback,
		&MainFrame::HandleKeyCallback,
		&MainFrame::HandleMouseClickCallback,
		NULL);
}


MainFrame::~MainFrame()
{
	XPLMDestroyWindow(gWindow);
}

/*
* MyDrawingWindowCallback
*
* This callback does the work of drawing our window once per sim cycle each time
* it is needed.  It dynamically changes the text depending on the saved mouse
* status.  Note that we don't have to tell X-Plane to redraw us when our text
* changes; we are redrawn by the sim continuously.
*
*/
void MainFrame::DrawWindowCallback(
	XPLMWindowID         inWindowID,
	void *               inRefcon)
{
	int		left, top, right, bottom;
	float	color[] = { 1.0, 1.0, 1.0 }; 	/* RGB White */
	if (inWindowID != m_ptMainFrame->gWindow) return;
											/* First we get the location of the window passed in to us. */
	XPLMGetWindowGeometry(inWindowID, &left, &top, &right, &bottom);

	/* We now use an XPLMGraphics routine to draw a translucent dark
	* rectangle that is our window's shape. */
	XPLMDrawTranslucentDarkBox(left, top, right, bottom);

	/* Finally we draw the text into the window, also using XPLMGraphics
	* routines.  The NULL indicates no word wrapping. */
	XPLMDrawString(color, left + 5, top - 20,
		(char*)(m_ptMainFrame->gClicked ? "Je suis le futur plugin FFSTracker" : "XFFSTracker"), NULL, xplmFont_Basic);

}

/*
* MyHandleKeyCallback
*
* Our key handling callback does nothing in this plugin.  This is ok;
* we simply don't use keyboard input.
*
*/
void MainFrame::HandleKeyCallback(
	XPLMWindowID         inWindowID,
	char                 inKey,
	XPLMKeyFlags         inFlags,
	char                 inVirtualKey,
	void *               inRefcon,
	int                  losingFocus)
{
}

/*
* MyHandleMouseClickCallback
*
* Our mouse click callback toggles the status of our mouse variable
* as the mouse is clicked.  We then update our text on the next sim
* cycle.
*
*/
int MainFrame::HandleMouseClickCallback(
	XPLMWindowID         inWindowID,
	int                  x,
	int                  y,
	XPLMMouseStatus      inMouse,
	void *               inRefcon)
{
	/* If we get a down or up, toggle our status click.  We will
	* never get a down without an up if we accept the down. */
	if ((inMouse == xplm_MouseDown) || (inMouse == xplm_MouseUp))
		m_ptMainFrame->gClicked = 1 - m_ptMainFrame->gClicked;

	/* Returning 1 tells X-Plane that we 'accepted' the click; otherwise
	* it would be passed to the next window behind us.  If we accept
	* the click we get mouse moved and mouse up callbacks, if we don't
	* we do not get any more callbacks.  It is worth noting that we
	* will receive mouse moved and mouse up even if the mouse is dragged
	* out of our window's box as long as the click started in our window's
	* box. */
	return 1;
}
