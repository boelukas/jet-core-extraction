#include <QApplication>
#include <vtkDebugLeaks.h>
#include "MainWindow.h"


// ---------------------------------------
int main(int argc, char* argv[])
{
#ifdef WIN32
	_CrtSetDbgFlag(
		_CRTDBG_ALLOC_MEM_DF | 
		_CRTDBG_LEAK_CHECK_DF);
#endif
	{
		// create the application
		QApplication app(argc, argv);

		// create the main window
		MainWindow window;

		// show the window and run the application
		window.show();
		int ret = app.exec();
		vtkDebugLeaks::PrintCurrentLeaks();
		return ret;
	}
}
