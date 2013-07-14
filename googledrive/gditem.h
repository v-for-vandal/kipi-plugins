#ifndef GDITEM_H
#define GDITEM_H

//Qt includes
#include <QString>

namespace KIPIGoogleDrivePlugin{

class GDPhoto{

public:
    GDPhoto(){
        //
    }

    QString title;
    QString description;
};

class GDFolder{

public:
    GDFolder(){
        //
    }
    QString title;

};
}

#endif // GDITEM_H
