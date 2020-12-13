#include "reactive2.h"

class eObject;

class eLink : public QJsonObject{
private:
    eObject* m_start;
    eObject* m_end;
};

class eObject : public QJsonObject{

};

class document{


};

static rea::regPip<QQmlApplicationEngine*> init_gui_main([](rea::stream<QQmlApplicationEngine*>* aInput) {
    static document doc;
    aInput->data()->load(QUrl(QStringLiteral("qrc:/GUI/main.qml")));
    aInput->out();
}, rea::Json("name", "loadMain"));
