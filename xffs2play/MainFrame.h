#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "singleton/webmanager.h"

class MainFrame : public QObject
{
public:
	static MainFrame* Create();
	static void Kill();
private:

	static MainFrame* m_ptMainFrame;

	MainFrame();
	~MainFrame();

	XPLMWindowID	gWindow = NULL;
	int				gClicked = 0;

	static void DrawWindowCallback(XPLMWindowID inWindowID,void* inRefcon);

	static void HandleKeyCallback(
		XPLMWindowID         inWindowID,
		char                 inKey,
		XPLMKeyFlags         inFlags,
		char                 inVirtualKey,
		void *               inRefcon,
		int                  losingFocus);

	static int HandleMouseClickCallback(
		XPLMWindowID         inWindowID,
		int                  x,
		int                  y,
		XPLMMouseStatus      inMouse,
		void *               inRefcon);
    CWebManager*    WebManager;
};

#endif
