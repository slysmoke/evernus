
#include <QColor>
#include <QSettings>

#include "UISettings.h"


namespace Evernus {

	namespace DarkModeColor {


		QColor green() {

			QSettings settings;

			if (settings.value(UISettings::mDarkModeKey, UISettings::mDarkModeDefault).toBool())
			{
				return QColor{ Qt::green };
			}
			else {
				return QColor{ Qt::darkGreen };
			}


	}

		QColor orderHighlight() {

			QSettings settings;

			if (settings.value(UISettings::mDarkModeKey, UISettings::mDarkModeDefault).toBool())
			{
				return QColor{ Qt::blue };
			}
			else {
				return QColor{ Qt::yellow };
			}


		}
	
		


	}

}